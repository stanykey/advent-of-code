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

namespace std {
    template<typename First, typename Second>
    std::istream& operator>>(std::istream& in, std::pair<First, Second>& pair) {
        return in >> pair.first >> pair.second;
    }
}  // namespace std

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
using Interval = std::pair<std::uint64_t, std::uint64_t>;

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
    using Table = std::vector<std::tuple<Interval, std::uint64_t>>;

    Table table_;
    FieldPtr from_;
    FieldPtr to_;
};


std::uint64_t find_nearest_location(std::istream& plan_document) {
    auto line = io::read<std::string>(plan_document);
    if (!line.starts_with("seeds:")) {
        throw std::invalid_argument("invalid input steam");
    }

    std::vector<Interval> seeds_rages;
    std::copy(
        std::istream_iterator<Interval>(plan_document), std::istream_iterator<Interval>(),
        std::back_inserter(seeds_rages)
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
        }
    }

    std::vector<std::uint64_t> min_locations;
    std::transform(
        seeds_rages.cbegin(), seeds_rages.cend(), std::back_inserter(min_locations),
        [&mappers](const Interval& pair) {
            std::vector<SeedInfo> seeds;
            for (auto i = 0; i != pair.second; i++) {
                seeds.push_back(SeedInfo{.id = pair.first + i});
            }

            mappers.at("seed-to-soil").lookup(seeds);
            mappers.at("soil-to-fertilizer").lookup(seeds);
            mappers.at("fertilizer-to-water").lookup(seeds);
            mappers.at("water-to-light").lookup(seeds);
            mappers.at("light-to-temperature").lookup(seeds);
            mappers.at("temperature-to-humidity").lookup(seeds);
            mappers.at("humidity-to-location").lookup(seeds);

            return std::min_element(seeds.cbegin(), seeds.cend(), [](const SeedInfo& lhs, const SeedInfo& rhs) {
                return lhs.location < rhs.location;
            })->location;
        }
    );

    return *std::min_element(min_locations.cbegin(), min_locations.cend());
}

int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-5\input.txt)");

    const auto result = find_nearest_location(document);
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
