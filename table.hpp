#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

//===================================================================
// Stany filozofa

enum class State {Thinking, Eating};
//===================================================================
//======================================================================
// Cały "monitor" procesu
class Table{
    public:
        explicit Table(int n); //tylko jawnie

        void take_forks(int id);

        void put_forks(int id);

        void copy_fork_owner(std::vector<int>& out); //do wizualizacji -> fork_owner[i] = -1 jesli widelec wolny, jesli nie to id filo

    private:

        int N;

        std::vector<int> fork_owner;


        // mechanizm sprawiedliwości poprzez bilety
        long long next_ticket = 0; 
        std::vector<long long> ticket; //ticket id dla aktualnej próby jedzenia 
        std::vector<bool> waiting; //czy filozof czeka na wolne widelce

        //Synchronizacja
        std::mutex mtx;
        std::condition_variable cv;

        //do widelców
        int left_fork(int id) const;
        int right_fork(int id) const;

        int left_neighbor(int id)const;
        int right_neighbor(int id)const;

        //warunki do rozpoczęcia jedzenia
        bool forks_free_for(int id) const; //będzie pytać czy oba widelce wolne
        bool has_priority_over_neighbors(int id) const; //sprawdzi czy sąsiedzi nie mają "starszego/mocniejszego" ticketa

};