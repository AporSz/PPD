#!/usr/bin/env python3
import sys
import os
import random

def write_matrix(file_path, rows, cols, min_val=0, max_val=100):
    population = [str(x) for x in range(min_val, max_val + 1)]
    with open(file_path, "w") as f:
        f.write(f"{rows}\n")
        f.write(f"{cols}\n")
        for _ in range(rows):
            row_vals = random.choices(population, k=cols)
            f.write(" ".join(row_vals) + "\n")

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <n> <m> <k>", file=sys.stderr)
        sys.exit(1)

    try:
        n = int(sys.argv[1])
        m = int(sys.argv[2])
        k = int(sys.argv[3])
    except ValueError:
        print("Error: n, m, and k must be integers.", file=sys.stderr)
        sys.exit(1)

    if n <= 0 or m <= 0 or k <= 0:
        print("Error: Dimensions must be positive integers.", file=sys.stderr)
        sys.exit(1)

    # Determine paths relative to the project directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.dirname(script_dir)
    files_dir = os.path.join(base_dir, "files")
    os.makedirs(files_dir, exist_ok=True)

    matrix_f_path = os.path.join(files_dir, "MatrixF.txt")
    convolution_m_path = os.path.join(files_dir, "ConvolutionM.txt")

    write_matrix(matrix_f_path, n, m)
    write_matrix(convolution_m_path, k, k)

    print(f"Generated Matrix F ({n}x{m}) in {matrix_f_path}")
    print(f"Generated Convolution Matrix C ({k}x{k}) in {convolution_m_path}")

if __name__ == "__main__":
    main()
