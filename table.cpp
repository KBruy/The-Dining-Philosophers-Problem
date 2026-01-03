#include "table.hpp"


//=========================================
Table::Table(int n)
    : N(n),
    fork_owner(n,-1),
    ticket(n,0),
    waiting(n,false){}
//==============================================
//==========================================
//Indexowanie widelców i sąsiadów

int Table::left_fork(int id) const{
    return id;
}

int Table::right_fork(int id) const{
    return (id + 1)%N;

}

int Table::left_neighbor(int id) const{
    return (id + N - 1) % N;

}

int Table::right_neighbor(int id) const{
    return (id + 1)%N;
}
//=========================================
//==========================================
//Flagi

bool Table::forks_free_for(int id) const {
    return fork_owner[left_fork(id)] == -1 && fork_owner[right_fork(id)] == -1;
}

bool Table::has_priority_over_neighbors(int id) const { 
    //jeśli sąsiad też czeka i ma mniejszy/mocnejeszy bilet to ma pierwszeństwo ,,żeby nie zagłodzić

    const int L = left_neighbor(id);
    const int R = right_neighbor(id);

    //jeśli lewy czeka i ma mocniejkszy bilet to ja czekam
    if (waiting[L]) {
        if (ticket[L] < ticket[id]) 
        return false;
        if (ticket[L] == ticket[id] && L < id)  //gdyby był remis (raczej niemożliwe)
        return false;
    }

    //jeśliprawy czeka i ma mocniejszy bilet to czekam

    if (waiting[R]) {
        if (ticket[R] < ticket[id]) 
        return false;
    if (ticket[R] == ticket[id] && R < id)
        return false;
    }
    return true;
}
//=========================================================================
//==========================================================================
// wejście do jedzenia
// bierzemy widelce lewy i prawy raz po drugim ( nie ma sytuacji że filozof podnosi lewy i nie zje za jakiś moment)
// jak się nie da to czeka

void Table::take_forks(int id) {
    std::unique_lock<std::mutex> lock(mtx);

    //wewnętrzne zgłoszenie że czeka na jedzenie
    waiting[id] = true;
    ticket[id] = next_ticket++;

    //dwa warunki: oba widelce wolne i priorytet nad sąsiadamni (nikt nie czeka dłużej niż on sam)
    cv.wait(lock, [&] {
        return forks_free_for(id) && has_priority_over_neighbors(id);
    });

    //Rezerwacja obu widelców atomowo pod mutexem

    fork_owner[left_fork(id)] = id;
    fork_owner[right_fork(id)] = id;

    //Już nie czeka -> zaczyna jeść
    waiting[id] = false;

}

//=================================================================
//============================================================
//Koniec jedzenia, oddajemy widelce i budzimy innych

void Table::put_forks(int id) {
    std::lock_guard<std::mutex> lock(mtx);

    fork_owner[left_fork(id)] = -1;
    fork_owner[right_fork(id)] = -1;

   cv.notify_all();
}
//==============================================================
//==============================================================
//"snapshot" do ui

void Table::copy_fork_owner(std::vector<int>& out) {
    std::lock_guard<std::mutex> lock(mtx);
    out = fork_owner;
}