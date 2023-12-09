#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace parsing {
    constexpr std::string_view DIGITS = R"(0123456789)";

    constexpr auto is_single_digit(char symbol) -> bool {
        return std::find(DIGITS.cbegin(), DIGITS.cend(), symbol) != DIGITS.cend();
    };

    const std::map<std::string_view, std::uint16_t> DIGITS_MAP{
        {"0", 0},    {"1", 1},   {"2", 2},     {"3", 3},     {"4", 4},
        {"5", 5},    {"6", 6},   {"7", 7},     {"8", 8},     {"9", 9},

        {"zero", 0}, {"one", 1}, {"two", 2},   {"three", 3}, {"four", 4},
        {"five", 5}, {"six", 6}, {"seven", 7}, {"eight", 8}, {"nine", 9},
    };

    template<typename Prefix>
        requires std::is_same_v<Prefix, char> || std::is_same_v<Prefix, const char*>
              || std::is_same_v<Prefix, std::string_view>
    std::vector<std::string_view> get_partial_matches(Prefix prefix) {
        std::vector<std::string_view> candidates;
        for (const auto& [word, _] : DIGITS_MAP) {
            if (word.starts_with(prefix)) {
                candidates.emplace_back(word);
            }
        }
        return candidates;
    }

    std::uint16_t to_digit(char digit) {
        if ((digit < '0') || (digit > '9')) {
            throw std::invalid_argument("unknown digit char");
        }
        return digit - '0';
    }

    std::uint16_t to_digit(std::string_view word) {
        const auto it = DIGITS_MAP.find(word);
        if (it == DIGITS_MAP.cend()) {
            throw std::invalid_argument("unknown digit spelling");
        }

        return it->second;
    }

    std::uint16_t get_first_digit(std::string_view line) {
        const auto digit_start = [](char symbol) -> bool {
            static const std::set<char> Filter{
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'z', 'o', 't', 'f', 's', 'e', 'n',
            };
            return Filter.contains(symbol);
        };

        auto cursor = std::find_if(line.cbegin(), line.cend(), digit_start);
        while (cursor != line.cend()) {
            if (is_single_digit(*cursor)) {
                return to_digit(*cursor);
            }

            const auto candidates = get_partial_matches(*cursor);
            const auto it         = std::find_if(candidates.cbegin(), candidates.cend(), [&](const std::string_view& word) {
                if (word.size() > std::distance(cursor, line.cend())) {
                    return false;
                }

                const auto end = cursor + static_cast<std::string_view::difference_type>(word.size());
                return std::search(cursor, end, word.cbegin(), word.cend()) != end;
            });

            if (it != candidates.cend()) {
                return to_digit(*it);
            }

            cursor = std::find_if(cursor + 1, line.cend(), digit_start);
        }

        return 0;
    }

    std::uint16_t get_last_digit(std::string_view line) {
        std::vector<std::pair<std::string_view, std::string_view::difference_type>> results;
        for (const auto& [word, _] : DIGITS_MAP) {
            const auto it = std::find_end(line.cbegin(), line.cend(), word.cbegin(), word.cend());
            if (it != line.cend()) {
                results.emplace_back(word, std::distance(line.cbegin(), it));
            }
        }

        if (results.empty()) {
            return 0;
        }

        const auto it = std::max_element(results.cbegin(), results.cend(), [](auto&& lhs, auto&& rhs) {
            return lhs.second < rhs.second;
        });
        return to_digit(it->first);
    }
}  // namespace parsing

std::vector<std::uint16_t> get_calibrations(std::istream& in) {
    std::vector<std::uint16_t> calibrations;

    std::string line;
    while (std::getline(in, line)) {
        const auto first_digit = parsing::get_first_digit(line);
        const auto last_digit  = parsing::get_last_digit(line);
        const auto number      = first_digit * 10 + last_digit;  // NOLINT: concatenate two single digits into one number
        calibrations.emplace_back(number);

        std::cout << line << " -> " << number << std::endl;
    }
    return calibrations;
}

int main() {
    std::ifstream document(R"(input.txt)");

    const auto calibrations = get_calibrations(document);
    const auto result       = std::accumulate(calibrations.begin(), calibrations.end(), std::uint32_t{0});
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
