#!/usr/bin/env python3
import argparse
import random
from pathlib import Path


def positive_length(value):
    try:
        length = int(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("lengths must be positive integers") from error
    if length <= 0:
        raise argparse.ArgumentTypeError("lengths must be positive integers")
    return length


def write_number(path, length):
    with path.open("w", encoding="ascii") as output:
        output.write(f"{length}\n")
        for index in range(length):
            if index:
                output.write(" ")
            digit = random.randint(1, 9) if index == length - 1 else random.randint(0, 9)
            output.write(str(digit))
        output.write("\n")


def main():
    parser = argparse.ArgumentParser(
        description="Generate two random digit files for the HW4 addition task."
    )
    parser.add_argument("length1", type=positive_length, help="digit count for number1.txt")
    parser.add_argument("length2", type=positive_length, help="digit count for number2.txt")
    args = parser.parse_args()

    output_dir = Path(__file__).resolve().parent
    number1_path = output_dir / "number1.txt"
    number2_path = output_dir / "number2.txt"
    write_number(number1_path, args.length1)
    write_number(number2_path, args.length2)

    print(f"Generated {args.length1}-digit number in {number1_path}")
    print(f"Generated {args.length2}-digit number in {number2_path}")


if __name__ == "__main__":
    main()
