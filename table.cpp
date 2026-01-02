#include "table.hpp"

Table::Table(int n)
    : N(n),
    fork_owner(n, -1),
    state(n, State::Thinking){}


void Table::take_forks(int id) {


    std::unique_lock<std::mutex> lock(mtx);

    state[id] = State::Hungry;
    queue.push_back(id);

    //Czekamy aż: będziemy pierwsi w kolerjce i obwa widelace są wolne

    cv.wait(lock, [&] {
        return queue.front() == id && forks_free_for(id);
    });


    //Rezerwacja dwóch widelcy [pod mutexem]

    fork_owner[left(id)] = id;
    fork_owner[right(id)] = id;
    
    state[id] = State::Eating;
}

void Table::put_forks(int id) {
    std::lock_guard<std::mutex> lock(mtx); // Blokuje mutex na czas funkcji, chroni współdzielone dane stołu

    fork_owner[left(id)] = -1;
    fork_owner[right(id)] = -1;

    state[id] = State::Thinking;

    //schodzimy z kolejki
    if (!queue.empty() && queue.front() == id){
        queue.pop_front();
    }

    //budzimy innych
    cv.notify_all();

}

int Table::left(int id) const{
    return id;
}

int Table::right(int id) const{
    return (id + 1) % N;
}

bool Table::forks_free_for(int id) const {
    return fork_owner[left(id)] == -1 && fork_owner[right(id)] == -1;
    
}

  