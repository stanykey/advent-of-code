#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
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

    template<typename T>
    std::vector<T> read_sequence(std::istream& input, bool has_prefix = false) {
        std::string line;
        if (!std::getline(input, line)) {
            return {};
        }

        std::istringstream reader(std::move(line));
        if (has_prefix) {
            std::ignore = read<std::string>(reader);
        }

        std::vector<T> sequence;
        std::copy(std::istream_iterator<T>(reader), std::istream_iterator<T>(), std::back_inserter(sequence));
        return sequence;
    }
}  // namespace io

namespace core {
    template <typename Integer>
    class number_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Integer;
        using difference_type = Integer;
        using pointer = Integer*;
        using reference = Integer&;

        explicit number_iterator(value_type current)
            : current_(current) {}

        value_type operator*() const {
            return current_;
        }

        number_iterator& operator++() {
            ++current_;
            return *this;
        }

        number_iterator operator++(int) {
            number_iterator temp = *this;
            ++(*this);
            return temp;
        }

        number_iterator& operator--() {
            --current_;
            return *this;
        }

        number_iterator operator--(int) {
            number_iterator temp = *this;
            --(*this);
            return temp;
        }

        number_iterator& operator+=(value_type n) {
            current_ += n;
            return *this;
        }

        number_iterator& operator-=(value_type n) {
            current_ -= n;
            return *this;
        }

        difference_type operator-(const number_iterator& other) const {
            return current_ - other.current_;
        }

        friend number_iterator operator+(number_iterator it, value_type n) {
            return (it += n);
        }

        friend number_iterator operator-(number_iterator it, value_type n) {
            return (it += n);
        }

        int operator[](value_type n) const {
            return current_ + n;
        }

    public:
        // Comparison operators
        bool operator==(const number_iterator& other) const {
            return current_ == other.current_;
        }
        bool operator!=(const number_iterator& other) const {
            return current_ != other.current_;
        }
        bool operator<(const number_iterator& other) const {
            return current_ < other.current_;
        }
        bool operator>(const number_iterator& other) const {
            return current_ > other.current_;
        }
        bool operator<=(const number_iterator& other) const {
            return current_ <= other.current_;
        }
        bool operator>=(const number_iterator& other) const {
            return current_ >= other.current_;
        }

    private:
        value_type current_;
    };

    template <typename Integer>
    number_iterator<Integer> make_number_iterator(Integer value) {
        return number_iterator<Integer>(value);
    }
}  // namespace core

struct RaceRecord {
    std::size_t duration = 0;
    std::size_t distance = 0;
};

std::vector<RaceRecord> load_races_records(std::istream& document) {
    const auto durations = io::read_sequence<std::size_t>(document, true);
    const auto distances = io::read_sequence<std::size_t>(document, true);
    if (durations.empty() || durations.size() != distances.size()) {
        throw std::invalid_argument("input data is corrupted");
    }

    std::vector<RaceRecord> records;
    records.reserve(durations.size());
    for (auto i = 0u; i != durations.size(); i++) {
        records.emplace_back(durations[i], distances[i]);
    }
    return records;
}

std::size_t simulate_race(std::size_t charge_duration, std::size_t total_duration) {
    const auto speed = charge_duration;
    const auto travel_time = total_duration - charge_duration;
    return speed * travel_time;
}

std::pair<std::size_t, std::size_t> min_max_charge_time(const RaceRecord& record) {
    const auto start = core::make_number_iterator<std::size_t>(0);
    const auto end = start + record.duration;
    const auto it = std::upper_bound(start, end, record.distance, [&record](std::size_t distance, std::size_t charge_time) {
        return simulate_race(charge_time, record.duration) > distance;
    });

    return {*it, record.duration - *it};
}


std::vector<std::pair<std::size_t, std::size_t>> determinate_winning_variants(const std::vector<RaceRecord>& records) {
    std::vector<std::pair<std::size_t, std::size_t>> winning_variants;

    std::transform(
        records.cbegin(), records.cend(), std::back_inserter(winning_variants),
        [](const RaceRecord& record) { return min_max_charge_time(record); }
    );

    return winning_variants;
}


int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-6\input.txt)");

    const auto races_records = load_races_records(document);
    const auto winning_variants = determinate_winning_variants(races_records);
    const auto result = std::transform_reduce(
        winning_variants.cbegin(), winning_variants.cend(), std::size_t{1}, std::multiplies{},
        [](const auto& pair) { return pair.second - pair.first + 1; }
    );
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
