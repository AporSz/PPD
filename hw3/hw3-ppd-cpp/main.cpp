#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

#include "model/Matrix.h"
#include "model/DynamicMatrix.h"

#include "executors/ConvolutionExecutor.h"
#include "executors/SequentialExecutor.h"
#include "executors/HorizontalExecutor.h"
#include "executors/VerticalExecutor.h"

using namespace std;
using namespace std::chrono;

Matrix* F = nullptr;
Matrix* C = nullptr;
int p = 4;

ConvolutionExecutor* getExecutorType(const string& type) {
    string t = type;
    transform(t.begin(), t.end(), t.begin(), ::tolower);

    if (t == "s") return new SequentialExecutor(F, C);
    if (t == "h") return new HorizontalExecutor(F, C);
    if (t == "v") return new VerticalExecutor(F, C);

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

void timeAndExecute(ConvolutionExecutor* executor, SequentialExecutor* seq, const string& outputPath, const string& sequentialPath, const string& matrixF_path) {
    bool seqExists = filesystem::exists(sequentialPath);
    if (!seqExists && dynamic_cast<SequentialExecutor*>(executor) == nullptr) {
        try {
            seq->executeTask(1);
            F->toFile(sequentialPath);
            delete F;
            F = Matrix::fromFile(matrixF_path);
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

    if (dynamic_cast<SequentialExecutor*>(executor) != nullptr) {
        F->toFile(sequentialPath);
    } else {
        F->toFile(outputPath);
        if (!verify(sequentialPath, outputPath)) {
            throw runtime_error("Incorrect solution");
        }
    }

    delete F;
    F = Matrix::fromFile(matrixF_path);

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
    if (argc != 3) {
        cerr << "Error: Please provide exactly two arguments: <threads> <type>\n";
        return 1;
    }

    string type;

    try {
        p = stoi(argv[1]);
        if (p <= 0) throw invalid_argument("Number of threads must be positive");
        type = argv[2];
        if (type.length() != 1) throw invalid_argument("Invalid executor type");
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    string matrixF_path = resolvePath("../files/MatrixF.txt");
    string convolutionM_path = resolvePath("../files/ConvolutionM.txt");
    string sequentialPath = resolvePath("../files/sequential.txt");
    string outputPath = resolvePath("../files/output.txt");

    try {
        F = Matrix::fromFile(matrixF_path);
        C = Matrix::fromFile(convolutionM_path);
        if (C->getN() != C->getM() || C->getN() % 2 == 0) {
            throw invalid_argument("Convolution matrix must be square with an odd dimension.");
        }

        auto* seq = new SequentialExecutor(F, C);
        ConvolutionExecutor* executor = getExecutorType(type);

        timeAndExecute(executor, seq, outputPath, sequentialPath, matrixF_path);

        delete executor;
        delete seq;
        delete C;
        delete F;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}