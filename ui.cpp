#include "ui.hpp"
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

static const char* ui_state_str(State s) {
    switch (s) {
        case State::Thinking: return "THINK";
        case State::Eating:   return "EAT";
        default:              return "?";
    }
}

static int count_eating(const std::vector<State>& st) {
    int c = 0;
    for (auto s : st) if (s == State::Eating) ++c;
    return c;
}

void ui_thread(UiShared* shared) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    // Kolory (jeśli terminal wspiera)
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE,  -1); // normal
        init_pair(2, COLOR_GREEN,  -1); // EAT
        init_pair(3, COLOR_CYAN,   -1); // header
        init_pair(4, COLOR_YELLOW, -1); // info
    }

    const int refresh_ms = 50;

    while (shared->running.load()) {
        // snapshot stanów filozofów + posiłków
        std::vector<State> st;
        std::vector<int> meals;
        {
            std::lock_guard<std::mutex> lock(shared->mtx);
            st = shared->state;
            meals = shared->meals_done;
        }

        // snapshot widelców z Table (oddzielnie, bez łapania ui.mtx)
        std::vector<int> forks;
        if (shared->table) {
            shared->table->copy_fork_owner(forks);
        }

        const int eating_now = count_eating(st);
        const int max_parallel = shared->N / 2; // teoretycznie max ~ floor(N/2)

        int cols = getmaxx(stdscr);


        erase();

        // ===== HEADER =====
        if (has_colors()) attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(0, 0, "Ucztujacy filozofowie |  N=%d  M=%d", shared->N, shared->M);
        if (has_colors()) attroff(COLOR_PAIR(3) | A_BOLD);

        if (has_colors()) attron(COLOR_PAIR(4));
        mvprintw(1, 0, "EATING: %d / ~%d", eating_now, max_parallel);
        // mvprintw(2, 0, "");
        if (has_colors()) attroff(COLOR_PAIR(4));

        // ===== LISTA FILOZOFOW =====
        int start_row = 4;
        mvprintw(start_row - 1, 0, "Filozofowie:");
        for (int i = 0; i < shared->N; ++i) {
            const char* label = ui_state_str(st[i]);

            // kolor EAT
            if (has_colors() && st[i] == State::Eating) attron(COLOR_PAIR(2) | A_BOLD);

            mvprintw(start_row + i, 0, "F%-2d  %-5s   posilki: %2d/%2d",
                     i, label, meals[i], shared->M);

            if (has_colors() && st[i] == State::Eating) attroff(COLOR_PAIR(2) | A_BOLD);
        }

        // ===== WIDELCE =====
        int fork_row = start_row + shared->N + 2;
        mvprintw(fork_row - 1, 0, "Widelce (index -> owner):  [-] wolny, [F#] trzymany");

        if (!forks.empty()) {
            int r = fork_row;
            int c = 0;

            for (int i = 0; i < shared->N; ++i) {
                std::string token;
                if (forks[i] == -1) token = "[" + std::to_string(i) + ":-] ";
                else token = "[" + std::to_string(i) + ":F" + std::to_string(forks[i]) + "] ";

                // zawijanie linii
                if (c + (int)token.size() >= cols) {
                    r++;
                    c = 0;
                }
                mvprintw(r, c, "%s", token.c_str());
                c += (int)token.size();
            }
        } else {
            mvprintw(fork_row, 0, "(brak danych o widelcach)");
        }

        refresh();

        // obsługa klawiszy (opcjonalnie)
        timeout(0);               // getch nie blokuje
        int ch = getch();
        if (ch == 27) {           // ESC
            shared->running = false;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_ms));
    }

    endwin();
}
