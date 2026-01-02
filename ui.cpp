#include "ui.hpp"
#include <ncurses.h>
#include <chrono>
#include <thread>

//==========================================================================================
//zamienia wartość enuma na etykiete THINK HUNGRY LUB EAT
//zwraca wskaźnik do stringa używanego później w mvprintw
static const char* state_str(State s) {
    switch (s) {
        case State::Thinking: return "THINK";
        case State::Hungry: return "HUNGRY";
        case State::Eating: return "EAT";
        default:
            return "?";
    }
}
//=========================================================================================
//=========================================================================================
// st - stany filozofów
// meals - liczba zjedzonych posiłków przez każdego filo
// w lock_guard blokuje shared->mtx żeby nie robić raceingu w trakcie kopiowania, kopiuje shared->meals_done do meals
// po wyjściu odblokowyjuje mutex żeby nie trzymać mutexa w czasie pracy ncurses tylko trzymać kopie "lokalną"

void ui_thread(UiShared* shared) {

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    

    while (shared->running.load()){//robimy "snapshot" pod mutexem żeby rysowanie było łatwiejsze
    std::vector<State> st;
    std::vector<int> meals;

    {
        std::lock_guard<std::mutex> lock(shared->mtx);
        st = shared-> state;
        meals = shared -> meals_done;

    }

    erase();
    mvprintw(0, 0, "Ucztujacy filozofowie | N=%d M =%d", shared->N, shared->M);
    mvprintw(1, 0, "Stan: THINK/HUNGRY/EAT");


    for (int i = 0; i < shared ->N; ++i) {
        mvprintw(3+i, 0, "F%d: %-6s posilki: %d/%d", //3+i bo pierwsze 2 to nagłówki
                 i, state_str(st[i]),meals[i], shared->M);

    } 

    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

endwin(); //kończy tryb tekstowu i przywraca tryb terminala
}