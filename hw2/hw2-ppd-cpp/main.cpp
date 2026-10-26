#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

#include "model/Matrix.h"
#include "model/StaticMatrix.h"
#include "model/DynamicMatrix.h"

#include "executors/ConvolutionExecutor.h"
#include "executors/SequentialExecutor.h"
#include "executors/HorizontalExecutor.h"
#include "executors/VerticalExecutor.h"
#include "executors/BlockExecutor.h"
#include "executors/LinearExecutor.h"
#include "executors/CyclicExecutor.h"

using namespace std;
using namespace std::chrono;

Matrix* F = nullptr;
Matrix* C = nullptr;
Matrix* V = nullptr;
int p = 4;

ConvolutionExecutor* getExecutorType(const string& type) {
    string t = type;
    transform(t.begin(), t.end(), t.begin(), ::tolower);

    if (t == "s") return new SequentialExecutor(F, C, V);
    if (t == "h") return new HorizontalExecutor(F, C, V);
    if (t == "v") return new VerticalExecutor(F, C, V);
    if (t == "b") return new BlockExecutor(F, C, V);
    if (t == "l") return new LinearExecutor(F, C, V);
    if (t == "c") return new CyclicExecutor(F, C, V);

    throw invalid_argument("Invalid executor type: " + type);
}

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

void timeAndExecute(ConvolutionExecutor* executor, SequentialExecutor* seq, const string& outputPath, const string& sequentialPath, bool isSequential) {
    bool seqExists = filesystem::exists(sequentialPath);
    if (!seqExists && !isSequential) {
        try {
            seq->executeTask(1);
            V->toFile(sequentialPath);
            V->reAllocate();
        } catch (const exception& e) {
            throw runtime_error(e.what());
        }
    }

    auto t0 = high_resolution_clock::now();

    try {
        executor->executeTask(p);
    } catch (const exception& e) {
        throw runtime_error(e.what());
    }

    auto t1 = high_resolution_clock::now();

    V->toFile(outputPath);

    if (isSequential) {
        if (!seqExists) {
            V->toFile(sequentialPath);
        }
    } else {
        if (!verify(sequentialPath, outputPath)) {
            throw runtime_error("Incorrect solution");
        }
    }

    duration<double, milli> ms = t1 - t0;
    cout << ms.count() << "\n";
}

string resolvePath(const string& relPath) {
    if (filesystem::exists(relPath)) return relPath;
    if (relPath.rfind("../", 0) == 0 && filesystem::exists(relPath.substr(3))) {
        return relPath.substr(3);
    }
    return relPath;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Error: Please provide at least two arguments: <threads> <type> [alloc_type]\n";
        return 1;
    }

    string type;
    bool isStatic = false;

    try {
        p = stoi(argv[1]);
        if (p <= 0) throw invalid_argument("Number of threads must be positive");
        type = argv[2];
        if (type.length() != 1) throw invalid_argument("Invalid executor type");
        if (argc >= 4) {
            string alloc = argv[3];
            transform(alloc.begin(), alloc.end(), alloc.begin(), ::tolower);
            if (alloc == "s" || alloc == "static") {
                isStatic = true;
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    string matrixF_path = resolvePath("../files/MatrixF.txt");
    string convolutionM_path = resolvePath("../files/ConvolutionM.txt");
    string sequentialPath = resolvePath("../files/sequential.txt");
    string outputPath = resolvePath("../files/output.txt");

    try {
        F = Matrix::fromFile(matrixF_path, isStatic);
        C = Matrix::fromFile(convolutionM_path, isStatic);
        if (C->getN() != C->getM() || C->getN() % 2 == 0) {
            throw invalid_argument("Convolution matrix must be square with an odd dimension.");
        }
        if (isStatic) {
            V = new StaticMatrix(F->getN(), F->getM());
        } else {
            V = new DynamicMatrix(F->getN(), F->getM());
        }

        auto* seq = new SequentialExecutor(F, C, V);
        ConvolutionExecutor* executor = getExecutorType(type);

        string t = type;
        transform(t.begin(), t.end(), t.begin(), ::tolower);
        bool isSequential = (t == "s");

        timeAndExecute(executor, seq, outputPath, sequentialPath, isSequential);

        delete executor;
        delete seq;
        delete V;
        delete C;
        delete F;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}