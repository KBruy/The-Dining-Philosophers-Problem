#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <string>

#include "table.hpp"
#include "ui.hpp"

//=========================================================
// zamiana argumentui na int i walidacja

bool parse_int(const char* s, int& out) {
    try{

        std::string str = s;
        size_t pos = 0;
        int val = std::stoi(str, &pos);
        if (pos != str.size()) //np czy arg to nie 123abc
        return false;
        out = val;
        return true;

    } catch(...) {
        return false;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Uzycie: " << argv[0] << " N M\n"
                  << "N = liczba filozofow (>=5)\n"
                  << "M = liczba posilkow na filozofa (>=1)\n";
        return 1;
    }

    int N = 0, M = 0;
    if (!parse_int(argv[1], N) || !parse_int(argv[2], M)) {
        std::cerr << "Blad: N i M musza byc liczbami calkowitymi.\n";
        return 1;
    }
    if (N < 5 || M < 1) {
        std::cerr << "Blad: N>=5 i M>=1.\n";
        return 1;
    }

    Table table(N); //monitor stołu

    UiShared ui(N,M);
    ui.table=&table;

    std::thread ui_t(ui_thread, &ui);//start wątku UI

//=====================
//start wątku filozofów

    std::vector<std::thread> threads;
    threads.reserve(N);

    auto set_state = [&](int id, State s) {
        std::lock_guard<std::mutex> lock(ui.mtx);
        ui.state[id] = s;
    };

    auto inc_meal = [&](int id) {
        std::lock_guard<std::mutex> lock(ui.mtx);
        ui.meals_done[id] += 1;
    };

    auto philosopher = [&](int id) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> think_ms(840, 1500);
        std::uniform_int_distribution<int> eat_ms(600, 1100);

        for (int meal = 1; meal <= M; ++meal) {
            // THINK
            set_state(id, State::Thinking);
            std::this_thread::sleep_for(std::chrono::milliseconds(think_ms(rng)));

            // Prośba o oba widelce (może blokować na condition_variable)
            table.take_forks(id);

            // EAT
            set_state(id, State::Eating);
            std::this_thread::sleep_for(std::chrono::milliseconds(eat_ms(rng)));

            // Oddanie widelców
            table.put_forks(id);
            inc_meal(id);
        }

        //Po wszystkim wracamy do Think
        set_state(id, State::Thinking);
    };

    for (int i=0; i<N;++i){
        threads.emplace_back(philosopher,i);
    }

    for (auto& t : threads) {
        t.join();
    }

    //koniec UI
    ui.running = false;
    ui_t.join();

    return 0;

}