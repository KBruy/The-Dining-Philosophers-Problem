#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <random>
#include <string>

std::mutex print_mtx;
//=============================================================================================
//bezpieczne wypisywanie jednej linijki pod mutexem
void safe_print(const std::string& s) {

    std::lock_guard<std::mutex> lock(print_mtx);
    std::cout << s << std::endl;
}
//=============================================================================================
//=============================================================================================
// Próba zamiany string na int, zwraca 1 dla dobrego a winik zapisuje do out 
bool parse_int(const char* s, int& out) {
    try {
        std::string str = s;
        size_t pos = 0;
        int val = std::stoi(str, &pos);

        //sprawdz czy string miał śmieci np 123ab

        if (pos != str.size()) 
        return false;

        out = val;
        return true;
    }
    catch (...){
        return false;
    }
}

//==============================================================================================
int main(int argc, char** argv) {
    // argc -> liczba argumentów (wraz z nazwą programu)
    // argv -> tablica napisów (argumentów)
    // np. dla .filozofowie 5 10    argc = 3     argv[1]="5", argv[2]="10"


    if (argc != 3) {
        std::cerr << "ERROR_1:\n" << "Wybrano: "<< argv[0] << " N M\n" << "N = liczba filozofow (>=5)\n" << "M = liczba posilkow jakie filozof ma zjesc (>=1)\n";
        return 1;
    }

    int N = 0;
    int M = 0;

    if(!parse_int(argv[1], N) || !parse_int(argv[2], M)) {
        std::cerr << "ERROR_2: N i M musza byc int";
        return 1;
    }

    if (N<5){
        std::cerr<< "ERROR_3: N musi byc >= 5\n";
        return 1;
    }

    if(M<1) {
        std::cerr<< "ERROR_4: M musi byc >=1\n";
        return 1;

    }

    safe_print("Start:\n N=" + std::to_string(N) + "\nM=" + std::to_string(M));


    std::vector<std::thread> philo_threads;
    philo_threads.reserve(N);


    //Funkcja wykonywana przez każdy wątek (filozofa)

    auto philosopher = [&](int id) {

        // generator losowy per-wątek (żeby różne czasy myślenia/jedzenia)

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> think_ms(200, 600);
        std::uniform_int_distribution<int> eat_ms(150, 450);

        for (int meal = 1; meal <= M; ++meal) {
            safe_print("F" + std::to_string(id)+" mysli (posilek " + std::to_string(meal) + "/" + std::to_string(M) + ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(think_ms(rng)));


            // na razie udajemy ze ma widelce i je je
            safe_print("F" + std::to_string(id) + " je    (posilek " + std::to_string(meal) + "/" + std::to_string(M) + ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(eat_ms(rng)));
        }

        safe_print("F" + std::to_string(id) + " skonczyl");
    };


    // Tworzymy N wątków
    for (int i = 0; i < N; ++i) {
        philo_threads.emplace_back(philosopher, i);
    }

    // Czekamy na zakończenie wszystkich wątków
    for (auto& thread : philo_threads) {
        thread.join();
    }

    safe_print("Koniec programu");
    return 0;

}
//================================================================================================