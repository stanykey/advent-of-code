#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace io {
    template<typename T>
    T read(std::istream& input) {
        T value{};
        input >> value;
        return value;
    }

    template<typename T>
    std::vector<T> read(std::istream& input, std::size_t count) {
        std::vector<T> result;

        result.reserve(count);
        while (count--) {
            auto value = read<T>(input);
            result.emplace_back(std::move(value));
        }

        return result;
    }

    std::string read_line(std::istream& input) {
        std::string line;
        std::getline(input, line);
        return line;
    }
}  // namespace io

struct Point {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t z = 0;
};

struct Brick {
    std::array<Point, 2> points;
};

std::vector<Brick> load_bricks(std::istream& in) {
    std::vector<Brick> bricks;
    while (in) {
        const auto line = io::read_line(in);

        std::smatch matches;
        static const std::regex pattern(R"((\d+),(\d+),(\d+)~(\d+),(\d+),(\d+))");
        if (!std::regex_match(line, matches, pattern)) {
            throw std::invalid_argument("Invalid input steam");
        }

        const std::array<Point, 2> points{
            Point{.x = std::stoi(matches[1]), .y = std::stoi(matches[2]), .z = std::stoi(matches[3])},
            Point{.x = std::stoi(matches[4]), .y = std::stoi(matches[5]), .z = std::stoi(matches[6])},
        };
        bricks.emplace_back(Brick{.points = points});
    }
    return bricks;
}

std::size_t count_unnecessary_bricks(const std::vector<Brick>& bricks) {
    return 0;
}

int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-22\test-data.txt)");

    const auto bricks = load_bricks(document);
    const auto result = count_unnecessary_bricks(bricks);
    std::cout << "The result is " << result << std::endl;
    return 0;
}
