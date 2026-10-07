#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

void init_random(vector<double>* v, const int n) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dis(0, 1000);

    v->reserve(n);

    for (int i = 0; i < n; i++) {
        v->push_back(dis(gen));
    }
}

void init_zeros(vector<double>* v, const int n) {
    for (int i = 0; i < n; i++) {
        v->push_back(0);
    }
}

void add(const vector<double>& a, const vector<double>& b, vector<double>& c) {
    for (int i = 0; i < a.size(); i++) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Error: Please provide exactly two arguments.\n";
        std::cerr << "Usage: " << argv[0] << " <argument1> <argument2>\n";
        return 1; // Exit with an error code
    }

    std::string firstArg = argv[1];
    std::string secondArg = argv[2];

    int n, nr_of_threads;

    try {
        n = stoi(firstArg);
        nr_of_threads = stoi(secondArg);
    }
    catch (exception& e) {
        std::cerr << e.what() << '\n';
    }

    vector<double> a, b, c;
    init_random(&a, n);
    init_random(&b, n);
    init_zeros(&c, n);

    auto begin = high_resolution_clock::now();
    add(a, b, c);
    auto end = high_resolution_clock::now();

    duration<double, std::milli> time = end - begin;

    cout << time.count() << "\n";

    return 0;
}