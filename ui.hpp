#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include "table.hpp"

struct UiShared{
    std::mutex mtx;
    
    std::vector<State> state; // stan filo, thinking/eating
    std::vector<int> meals_done; //ile każdy z filozofów już zjadł

    int N;
    int M;

    // podglądanie stanu widelców przez snapshot w Table
    Table* table= nullptr;


    std::atomic<bool> running{true}; //flaga do zatrzymania rysowania

    UiShared(int n, int m)
        : state(n, State::Thinking),
        meals_done(n,0),
        N(n),
        M(m) {}

};

// Funckja startowa wątku UI (ncurses)
void ui_thread(UiShared* shared);
