#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "model/Matrix.h"
#include "executors/ConvolutionExecutor.h"
#include "executors/HorizontalExecutor.h"
#include "executors/SequentialExecutor.h"
#include "executors/VerticalExecutor.h"

using namespace std;
using namespace std::chrono;

bool verify(const string& sequentialPath, const string& outputPath) {
    ifstream fSeq(sequentialPath);
    ifstream fOut(outputPath);
    if (!fSeq.is_open() || !fOut.is_open()) return false;

    string lineSeq, lineOut;
    while (getline(fSeq, lineSeq)) {
        if (!getline(fOut, lineOut)) return false;
        while (!lineSeq.empty() && (lineSeq.back() == '\r' || lineSeq.back() == ' ')) lineSeq.pop_back();
        while (!lineOut.empty() && (lineOut.back() == '\r' || lineOut.back() == ' ')) lineOut.pop_back();
        if (lineSeq != lineOut) return false;
    }
    return !getline(fOut, lineOut);
}

string resolvePath(const string& relPath) {
    if (filesystem::exists(relPath)) return relPath;
    if (relPath.rfind("../", 0) == 0 && filesystem::exists(relPath.substr(3))) {
        return relPath.substr(3);
    }
    return relPath;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Error: expected arguments <threads> <s|h|v>.\n";
        return 1;
    }

    int p;
    string type = argv[2];
    transform(type.begin(), type.end(), type.begin(), [](unsigned char c) {
        return static_cast<char>(tolower(c));
    });

    try {
        p = stoi(argv[1]);
        if (p <= 0) throw invalid_argument("Number of threads must be positive");
        if (type != "s" && type != "h" && type != "v") {
            throw invalid_argument("Executor must be 's', 'h', or 'v'");
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    const string inputPath = resolvePath("../files/date.txt");
    const string convolutionPath = resolvePath("../files/ConvolutionM.txt");
    const string sequentialPath = resolvePath("../files/sequential.txt");
    const string outputPath = resolvePath("../files/output.txt");

    try {
        unique_ptr<Matrix> F(Matrix::fromFile(inputPath));
        unique_ptr<Matrix> C(Matrix::fromFile(convolutionPath));
        if (C->getN() != 3 || C->getM() != 3) {
            throw invalid_argument("HW3 requires a 3x3 convolution matrix.");
        }
        if (type == "h" && p > F->getN()) {
            throw invalid_argument("Line-based thread count cannot exceed the number of rows.");
        }
        if (type == "v" && p > F->getM()) {
            throw invalid_argument("Line-based thread count cannot exceed the number of columns.");
        }

        unique_ptr<ConvolutionExecutor> executor;
        if (type == "s") {
            executor = make_unique<SequentialExecutor>(F.get(), C.get());
        } else if (type == "h") {
            executor = make_unique<HorizontalExecutor>(F.get(), C.get());
        } else {
            executor = make_unique<VerticalExecutor>(F.get(), C.get());
        }

        const auto t0 = high_resolution_clock::now();
        executor->executeTask(type == "s" ? 1 : p);
        const auto t1 = high_resolution_clock::now();

        F->toFile(outputPath);
        if (type == "s") {
            F->toFile(sequentialPath);
        } else if (!verify(sequentialPath, outputPath)) {
            throw runtime_error("Incorrect solution or missing sequential reference.");
        }

        duration<double, milli> ms = t1 - t0;
        cout << ms.count() << "\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
