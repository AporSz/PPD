#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <cmath>

using namespace std;
using namespace std::chrono;

bool load_input_file(const string& filename, int n, vector<double>& a, vector<double>& b) {
    ifstream in(filename);
    if (!in.is_open()) return false;
    int file_n;
    if (!(in >> file_n) || file_n != n) return false;

    a.resize(n);
    b.resize(n);
    for (int i = 0; i < n; i++) {
        if (!(in >> a[i])) return false;
    }
    for (int i = 0; i < n; i++) {
        if (!(in >> b[i])) return false;
    }
    return true;
}

void write_output_file(const string& filename, const vector<double>& c) {
    ofstream out(filename);
    out << c.size() << "\n";
    for (size_t i = 0; i < c.size(); i++) {
        out << fixed << setprecision(6) << c[i] << (i == c.size() - 1 ? "" : " ");
    }
    out << "\n";
}

bool check_files_equal(const string& file1, const string& file2) {
    ifstream f1(file1);
    ifstream f2(file2);
    if (!f1.is_open() || !f2.is_open()) return false;

    int n1, n2;
    if (!(f1 >> n1) || !(f2 >> n2) || n1 != n2) return false;

    double v1, v2;
    for (int i = 0; i < n1; i++) {
        if (!(f1 >> v1) || !(f2 >> v2)) return false;
        if (abs(v1 - v2) > 1e-4) return false;
    }
    return true;
}

void init_zeros(vector<double>* v, const int n) {
    v->clear();
    v->resize(n, 0.0);
}

void add(const vector<double>& a, const vector<double>& b, vector<double>& c) {
    for (size_t i = 0; i < a.size(); i++) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Error: Please provide exactly two arguments.\n";
        std::cerr << "Usage: " << argv[0] << " <argument1> <argument2>\n";
        return 1;
    }

    std::string firstArg = argv[1];
    std::string secondArg = argv[2];

    int n = 0, nr_of_threads = 0;

    try {
        n = stoi(firstArg);
        nr_of_threads = stoi(secondArg);
    }
    catch (exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    string input_filename = "input.txt";
    string output_filename = "output.txt";
    string seq_output_filename = "output_seq.txt";

    vector<double> a, b, c;
    if (!load_input_file(input_filename, n, a, b)) {
        std::cerr << "Error: Could not load input file '" << input_filename << "' with size " << n << ".\n";
        return 1;
    }

    init_zeros(&c, n);

    auto begin = high_resolution_clock::now();
    add(a, b, c);
    auto end = high_resolution_clock::now();

    write_output_file(output_filename, c);

    ifstream check_seq(seq_output_filename);
    if (!check_seq.is_open()) {
        write_output_file(seq_output_filename, c);
    } else {
        check_seq.close();
        if (!check_files_equal(output_filename, seq_output_filename)) {
            std::cerr << "Verification failed: Output file does not match sequential reference file.\n";
        }
    }

    duration<double, std::milli> time = end - begin;

    cout << time.count() << "\n";

    return 0;
}