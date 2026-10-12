#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

using namespace std;
using namespace std::chrono;

double* a;
double* b;
double* c ;
int n, p;
double dot_product = 0;
mutex product_mutex;

void hello(int id) {
    cout<<"Hello from thread "<<id<<endl;
}

void sequential(double* a, double* b, int n) {
    double sum = 0;
    auto t0 = chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    auto t1 = chrono::high_resolution_clock::now();

    chrono::duration<double, std::micro> time = t1 - t0;

    cout<<"Time taken for sequential execution: " << time.count()<<" microseconds"<<endl;
    cout<<"Sum = "<<sum<<endl;
}

void linear_worker(int id, int start, int end) {
    for (int i = start; i < end; i++) {
        c[id] += a[i] * b[i];

        //intentional race condition
        dot_product += a[i] * b[i];
    }
}

void linear_worker_mutex(int id, int start, int end) {
    for (int i = start; i < end; i++) {
        c[id] += a[i] * b[i];
    }

    //improvement over the linear_worker version since the partial sum is calculated
    //and added to the global sum at the end, not inside the for loop
    lock_guard<mutex> lock(product_mutex);
    dot_product += c[id];
}

void linear(void (*worker) (int id, int start, int end)) {
    int rest = n % p;
    int base = n / p;
    int start = 0, end = 0;

    vector<thread> threads;

    for (int i = 0; i < p; i++) {
        end = start + base + (i < rest ? 1 : 0);

        threads.emplace_back(worker, i, start, end);

        start = end;
    }

    for (auto &th : threads) {
        th.join();
    }
}

int main(int argc, char* argv[]) {
    vector<thread> threads;

    n = atoi(argv[1]);
    p = atoi(argv[2]);

    a = new double[n];
    b = new double[n];
    c = new double[p];

    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }

    // for (int i = 0; i < 20; i++) {
    //     threads.emplace_back(hello, i);
    // }

    // for (auto &th : threads) {
    //     th.join();
    // }

    sequential(a, b, n);

    auto t0 = chrono::high_resolution_clock::now();

    linear(linear_worker);

    auto t1 = chrono::high_resolution_clock::now();

    cout<<"Time taken for threaded (linear) execution (race condition): " << chrono::duration_cast<chrono::microseconds>(t1-t0).count()<<" microseconds"<<endl;
    cout<<"Sum = "<<dot_product<<endl;

    dot_product = 0;
    c = new double[p];

    t0 = chrono::high_resolution_clock::now();

    linear(linear_worker_mutex);

    t1 = chrono::high_resolution_clock::now();

    cout<<"Time taken for threaded (linear) execution (mutex): " << chrono::duration_cast<chrono::microseconds>(t1-t0).count()<<" microseconds"<<endl;
    cout<<"Sum = "<<dot_product<<endl;

    delete[] a;
    delete[] b;
    delete[] c;
}