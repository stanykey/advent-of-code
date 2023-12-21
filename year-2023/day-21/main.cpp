#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
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
}  // namespace io


class Garden {
private:
    static constexpr char PILE = '.';
    static constexpr char ROCK = '#';
    static constexpr char PLAYER = 'S';

    using Map = std::vector<std::string>;

    struct Point {
        int x = 0;
        int y = 0;

        bool operator<(const Point& other) const {
            return x == other.x ? y < other.y : x < other.x;
        }
    };

public:
    static Garden load(std::istream& in) {
        Garden garden;

        std::string line;
        while (std::getline(in, line) && !line.empty()) {
            garden.map_.emplace_back(std::move(line));
        }

        return garden;
    }

    [[nodiscard]] std::size_t count_end_plots(std::size_t steps) const {
        std::set<Point> current_positions{get_start_point()};
        while (steps--) {
            current_positions = determine_next_positions(current_positions);
        }
        return current_positions.size();
    }

private:
    Garden() = default;

    [[nodiscard]] Point get_start_point() const {
        for (std::size_t row = 0; row != map_.size(); row++) {
            const auto col = map_[row].find(PLAYER);
            if (col != std::string::npos) {
                return {static_cast<int>(row), static_cast<int>(col)};
            }
        }

        throw std::invalid_argument("the map is invalid");
    }

    [[nodiscard]] std::set<Point> determine_next_positions(const std::set<Point>& current_positions) const {
        std::set<Point> next_positions;
        const std::vector<Point> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

        for (const auto& pos : current_positions) {
            for (const auto& dir : directions) {
                Point next_pos = {pos.x + dir.x, pos.y + dir.y};
                if (next_pos.x >= 0 && next_pos.x < map_.size() && next_pos.y >= 0 && next_pos.y < map_[0].size()
                    && map_[next_pos.x][next_pos.y] != ROCK) {
                    next_positions.insert(next_pos);
                }
            }
        }

        return next_positions;
    };

private:
    Map map_;
};


int main() {
    std::ifstream document(R"(/home/slovygin/work/fun/adventofcode/year-2023/day-21/input.txt)");
    const auto garden = Garden::load(document);
    const auto steps = std::size_t{64};
    std::cout << "The result is " << garden.count_end_plots(steps) << std::endl;
    return 0;
}
