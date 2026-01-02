#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <deque>


//================================================================
//Stany filozofa
enum class State {Thinking, Hungry, Eating };

class Table {
    public:
        explicit Table(int n);  //explicit tylko wywołanie jawne

        //Filozof prosi o oba widelce (może czekać)
        void take_forks(int id);

        // Filozof oddaje oba widelce
        void put_forks(int id);


    private:
        int N; //ilosc filozofow

        //fork_owner[i] = -1 jesli widelec wolny, jesli nie to id filo
        std::vector<int> fork_owner;

        //stan filo (mysli/glodny/je)
        std::vector<State> state;

        //Synchronizacja / monitor

        std::mutex mtx;
        std::conditon_variable cv;

        //FIFO dla fairness (brak starvation)
        std::deque<int> quene;
        int left(int id) const;
        int right(int id) const;

        bool forks_free_for(int id) const;


};