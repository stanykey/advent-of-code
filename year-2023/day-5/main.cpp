#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
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

struct SeedRequirements {
    std::uint16_t soil = 0;
    std::uint16_t fertilizer = 0;
    std::uint16_t water = 0;
    std::uint16_t light = 0;
    std::uint16_t temperature = 0;
    std::uint16_t humidity = 0;
    std::uint16_t location = 0;
};

struct GardenPlan {
    std::unordered_map<std::uint32_t, SeedRequirements> plan;
};

std::vector<std::vector<std::uint16_t>> read_map_data(std::istream& in) {
    std::vector<std::vector<std::uint16_t>> data;

    std::string line;
    while (std::getline(in, line) && !line.empty()) {
        static constexpr std::size_t DataLineNumbersCount = 3;
        std::istringstream data_line(line);
        data.emplace_back(io::read<std::uint16_t>(data_line, DataLineNumbersCount));
    }

    return data;
}

GardenPlan load_garden_plan(std::istream& plan_document) {
    std::string line;

    std::getline(plan_document, line);
    if (line.starts_with("seeds:")) {
        throw std::invalid_argument("invalid input steam");
    }

    GardenPlan plan;
    std::transform(
        std::istream_iterator<std::uint32_t>(plan_document), std::istream_iterator<std::uint32_t>(),
        std::inserter(plan.plan, plan.plan.end()),
        [](std::uint32_t id) { return std::make_pair(id, SeedRequirements{}); }
    );

    std::unordered_map<std::string, std::vector<std::vector<std::uint16_t>>> raw_maps;
    while (std::getline(plan_document, line)) {
        static constexpr std::string_view MapSuffix = "map:";
        if (line.ends_with(MapSuffix)) {
            auto& data = raw_maps[line.substr(0, line.size() - (MapSuffix.size() + 1))];
            data = read_map_data(plan_document);
        }
    }

    return plan;
}

int main() {
    std::ifstream document(R"(input.txt)");

    const auto garden_plan = load_garden_plan(document);
    const auto result =
        std::min_element(garden_plan.plan.cbegin(), garden_plan.plan.cend(), [](const auto& lhs, const auto& rhs) {
            return lhs.second.location < rhs.second.location;
        })->second.location;
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
