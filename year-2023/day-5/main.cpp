#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
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

struct SeedInfo {
    std::uint64_t id = 0;
    std::uint64_t soil = 0;
    std::uint64_t fertilizer = 0;
    std::uint64_t water = 0;
    std::uint64_t light = 0;
    std::uint64_t temperature = 0;
    std::uint64_t humidity = 0;
    std::uint64_t location = 0;
};

using FieldPtr = std::uint64_t SeedInfo::*;


struct GardenPlan {
    std::vector<SeedInfo> seeds;
};

class Mapper {
public:
    Mapper(FieldPtr from, FieldPtr to)
        : from_(from)
        , to_(to) {}

    void load_table(std::istream& in) {
        std::string line;
        while (std::getline(in, line) && !line.empty()) {
            std::istringstream data_line(line);

            auto dest_start = io::read<std::uint64_t>(data_line);
            auto src_start = io::read<std::uint64_t>(data_line);
            auto range_size = io::read<std::uint64_t>(data_line);

            table_.emplace_back(std::make_pair(src_start, src_start + range_size - 1), dest_start);
        }
    }

    void lookup(std::vector<SeedInfo>& seeds) const {
        for (auto& seed : seeds) {
            const auto from = seed.*from_;
            const auto it =
                std::find_if(table_.cbegin(), table_.cend(), [&from](const std::tuple<Interval, std::uint64_t>& entry) {
                    const auto& [interval, _] = entry;
                    return interval.first <= from && from <= interval.second;
                });
            if (it == table_.cend()) {
                seed.*to_ = from;
            } else {
                const auto& [interval, value] = *it;
                seed.*to_ = value + from - interval.first;
            }
        }
    }

private:
    using Interval = std::pair<std::uint64_t, std::uint64_t>;
    using Table = std::vector<std::tuple<Interval, std::uint64_t>>;

    Table table_;
    FieldPtr from_;
    FieldPtr to_;
};


std::vector<SeedInfo> load_garden_plan(std::istream& plan_document) {
    auto line = io::read<std::string>(plan_document);
    if (!line.starts_with("seeds:")) {
        throw std::invalid_argument("invalid input steam");
    }

    std::vector<SeedInfo> seeds;
    std::transform(
        std::istream_iterator<std::uint64_t>(plan_document), std::istream_iterator<std::uint64_t>(),
        std::back_inserter(seeds), [](std::uint64_t id) { return SeedInfo{.id = id}; }
    );

    std::unordered_map<std::string, Mapper> mappers{
        {"seed-to-soil", Mapper(&SeedInfo::id, &SeedInfo::soil)},
        {"soil-to-fertilizer", Mapper(&SeedInfo::soil, &SeedInfo::fertilizer)},
        {"fertilizer-to-water", Mapper(&SeedInfo::fertilizer, &SeedInfo::water)},
        {"water-to-light", Mapper(&SeedInfo::water, &SeedInfo::light)},
        {"light-to-temperature", Mapper(&SeedInfo::light, &SeedInfo::temperature)},
        {"temperature-to-humidity", Mapper(&SeedInfo::temperature, &SeedInfo::humidity)},
        {"humidity-to-location", Mapper(&SeedInfo::humidity, &SeedInfo::location)},
    };

    plan_document.clear();
    while (std::getline(plan_document, line)) {
        static constexpr std::string_view MapSuffix = "map:";
        if (line.ends_with(MapSuffix)) {
            const auto name = line.substr(0, line.size() - (MapSuffix.size() + 1));
            auto& mapper = mappers.at(name);
            mapper.load_table(plan_document);
            mapper.lookup(seeds);
        }
    }

    return seeds;
}

int main() {
    std::ifstream document(R"(input.txt)");

    const auto garden_plan = load_garden_plan(document);
    const auto result =
        std::min_element(garden_plan.cbegin(), garden_plan.cend(), [](const auto& lhs, const auto& rhs) {
            return lhs.location < rhs.location;
        })->location;
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
