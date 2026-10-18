#include "Matrix.h"
#include "StaticMatrix.h"
#include "DynamicMatrix.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

void Matrix::toFile(const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }
    out << this->getN() << "\n";
    out << this->getM() << "\n";
    for (int i = 0; i < this->getN(); i++) {
        for (int j = 0; j < this->getM(); j++) {
            out << this->get(i, j) << " ";
        }
        out << "\n";
    }
}

string Matrix::toString() {
    stringstream ss;
    for (int i = 0; i < this->getN(); i++) {
        for (int j = 0; j < this->getM(); j++) {
            ss << this->get(i, j) << " ";
        }
        ss << "\n";
    }
    return ss.str();
}

bool Matrix::equals(Matrix* other) {
    if (other == this) return true;
    if (other == nullptr) return false;
    if (this->getN() != other->getN() || this->getM() != other->getM()) return false;

    for (int i = 0; i < this->getN(); i++) {
        for (int j = 0; j < this->getM(); j++) {
            if (this->get(i, j) != other->get(i, j)) {
                return false;
            }
        }
    }
    return true;
}

int Matrix::convolveAt(Matrix* F, Matrix* C, int i, int j) {
    int sum = 0;
    int half_size = C->getN() / 2;

    for (int k = -half_size; k <= half_size; k++) {
        for (int l = -half_size; l <= half_size; l++) {
            sum += F->get(i + k, j + l) * C->get(k + half_size, l + half_size);
        }
    }

    return sum;
}

Matrix* Matrix::fromFile(const string& filename, bool isStatic) {
    ifstream in(filename);
    if (!in.is_open()) {
        throw runtime_error("File doesn't exist: " + filename);
    }

    int n, m;
    if (!(in >> n >> m)) {
        throw runtime_error("Invalid file: " + filename);
    }

    Matrix* mat = nullptr;
    if (isStatic) {
        mat = new StaticMatrix(n, m);
    } else {
        mat = new DynamicMatrix(n, m);
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int val;
            if (!(in >> val)) {
                delete mat;
                throw runtime_error("Invalid file format: " + filename);
            }
            mat->set(val, i, j);
        }
    }

    return mat;
}
