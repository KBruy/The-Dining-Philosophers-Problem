#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include "table.hpp" // enum State


struct UiShared {
    std::mutex mtx;
    std::vector<State> state;
    std::vector<int> meals_done; //ile posiłków ukończone

    int N;
    int M;

    std::atomic<bool> running{true};

    UiShared(int n, int m)
        : state(n, State::Thinking), meals_done(n,0), N(n), M(m){}
};

void ui_thread(UiShared* shared);