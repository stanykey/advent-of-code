#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


namespace parsing {
    constexpr std::string_view DIGITS = R"(0123456789)";

    constexpr std::array<std::string_view, 10> DIGITS_WORDS{"zero", "one", "two",   "three", "four",
                                                            "five", "six", "seven", "eight", "nine"};

    constexpr auto is_single_digit(char symbol) -> bool {
        return std::find(DIGITS.cbegin(), DIGITS.cend(), symbol) != DIGITS.cend();
    };

    auto digit_start(char symbol) -> bool {
        const auto digit_it = std::find(DIGITS.cbegin(), DIGITS.cend(), symbol);
        if (digit_it != DIGITS.cend()) {
            return true;
        }

        const auto word_it = std::find_if(DIGITS_WORDS.cbegin(), DIGITS_WORDS.cend(), [symbol](const auto& word) {
            return word.front() == symbol;
        });

        return word_it != DIGITS_WORDS.cend();
    };

    template<typename Prefix>
        requires std::is_same_v<Prefix, char> || std::is_same_v<Prefix, const char*>
              || std::is_same_v<Prefix, std::string_view>
    std::vector<std::string_view> get_partial_matches(Prefix prefix) {
        std::vector<std::string_view> candidates;
        std::copy_if(
            DIGITS_WORDS.cbegin(), DIGITS_WORDS.cend(), std::back_inserter(candidates),
            [prefix](const std::string_view& word) { return word.starts_with(prefix); }
        );
        return candidates;
    }

    std::uint16_t to_digit(char digit) {
        if ((digit < '0') || (digit > '9')) {
            throw std::invalid_argument("unknown digit char");
        }
        return digit - '0';
    }

    std::uint16_t to_digit(std::string_view word) {
        const auto it = std::find(DIGITS_WORDS.cbegin(), DIGITS_WORDS.cend(), word);
        if (it == DIGITS_WORDS.cend()) {
            throw std::invalid_argument("unknown digit spelling");
        }

        return std::distance(DIGITS_WORDS.cbegin(), it);
    }

    std::vector<std::uint16_t> parse_digits(std::string_view string) {
        std::vector<std::uint16_t> digits;

        auto cursor = std::find_if(string.cbegin(), string.cend(), digit_start);
        while (cursor != string.cend()) {
            if (is_single_digit(*cursor)) {
                digits.emplace_back(to_digit(*cursor));
                cursor++;
            } else {
                const auto candidates = get_partial_matches(*cursor);
                const auto it = std::find_if(candidates.cbegin(), candidates.cend(), [&](const std::string_view& word) {
                    if (word.size() > std::distance(cursor, string.cend())) {
                        return false;
                    }

                    const auto end = cursor + static_cast<std::string_view::difference_type>(word.size());
                    return std::search(cursor, end, word.cbegin(), word.cend()) != end;
                });

                if (it != candidates.cend()) {
                    digits.emplace_back(to_digit(*it));
                    std::advance(cursor, it->size());
                } else {
                    cursor++;
                }
            }

            cursor = std::find_if(cursor, string.cend(), digit_start);
        }

        return digits;
    }
}  // namespace parsing

std::vector<std::uint16_t> get_calibrations(std::istream& in) {
    std::vector<std::uint16_t> calibrations;

    std::string line;
    while (std::getline(in, line)) {
        const auto digits = parsing::parse_digits(line);
        std::cout << line << " -> ";
        std::copy(digits.cbegin(), digits.cend(), std::ostream_iterator<std::uint16_t>(std::cout, ""));

        const auto first_digit = digits.empty() ? 0 : digits.front();
        const auto last_digit  = digits.empty() ? 0 : digits.back();
        const auto number      = first_digit * 10 + last_digit;  // NOLINT: concatenate two single digits into one number
        std::cout << " -> " << number << std::endl;

        calibrations.emplace_back(number);
    }
    return calibrations;
}

int main() {
    std::ifstream document(R"(input.txt)");

    const std::vector<std::uint16_t> calibrations = get_calibrations(document);
    const auto result = std::accumulate(calibrations.begin(), calibrations.end(), std::uint32_t{0});
    std::cout << "result value is " << result << std::endl;

    return 0;
}
