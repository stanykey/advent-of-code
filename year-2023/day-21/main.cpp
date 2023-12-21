#include <algorithm>
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
}  // namespace io


class Garden {
public:
    static constexpr char PILE = '.';
    static constexpr char ROCK = '#';
    static constexpr char PLAYER = 'S';

    using Map = std::vector<std::string>;

public:
    static Garden load(std::istream& in) {
        Garden garden;
        std::copy(
            std::istream_iterator<std::string>(in), std::istream_iterator<std::string>(in),
            std::back_inserter(garden.map_)
        );
        return garden;
    }

    std::size_t count_available_positions(std::size_t steps) const {
        return 0;
    }

private:
    Garden() = default;

    [[nodiscard]] std::pair<std::size_t, std::size_t> find_player() const {
        for (std::size_t row = 0; row != map_.size(); row++) {
            const auto col = map_[row].find(PILE);
            if (col != std::string::npos) {
                return {row, col};
            }
        }

        throw std::invalid_argument("the map is invalid");
    }

private:
    Map map_;
};


int main() {
    std::ifstream document(R"(/home/slovygin/work/fun/adventofcode/year-2023/day-21/test-data.txt)");
    const auto garden = Garden::load(document);
    const auto steps = std::size_t{64};
    std::cout << "The result is " << garden.count_available_positions(steps) << std::endl;
    return 0;
}
