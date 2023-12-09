#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

using namespace std::literals;


template<typename Integer>
Integer as_int(std::string_view str) {
    Integer value{};
    const auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec == std::errc::invalid_argument || ec == std::errc::result_out_of_range) {
        throw std::invalid_argument("Invalid conversion to integer.");
    }
    return value;
}

struct Scheme {
public:
    static constexpr std::string_view DIGITS = "0123456789";
    static inline const std::set<char> NOT_SYMBOLS{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};

    static Scheme load_scheme(std::istream& document) {
        Scheme scheme;

        std::string line;
        while (std::getline(document, line)) {
            scheme.data_.emplace_back(std::move(line));
        }

        return scheme;
    }

    [[nodiscard]] std::vector<std::uint32_t> get_parts_numbers() const {
        std::vector<std::uint32_t> parts_numbers;

        for (auto i = 0u; i != data_.size(); i++) {
            const std::string_view line = data_[i];

            auto [number, pos] = get_number(line, 0);
            while (!number.empty()) {
                if (is_valid_number(number, i, pos)) {
                    parts_numbers.emplace_back(as_int<std::uint32_t>(number));
                }

                std::tie(number, pos) = get_number(line, pos + number.size() + 1);
            }
        }

        return parts_numbers;
    }

private:
    static std::tuple<std::string_view, std::size_t> get_number(std::string_view str, std::size_t offset) {
        const auto start = str.find_first_of(DIGITS, offset);
        if (start == std::string_view::npos) {
            return {""sv, start};
        }

        const auto end = str.find_first_not_of(DIGITS, start);
        return {str.substr(start, end - start), start};
    }

    [[nodiscard]] bool is_valid_number(std::string_view number, std::size_t row, std::size_t col) const {
        std::vector<char> environment;

        environment.reserve((number.size() + 2) * 3);

        const auto start_scan_pos = col == 0 ? 0u : col - 1;
        const auto max_scan_length = number.size() + (col == 0 ? 1 : 2);
        const auto copy_row_part = [&environment](const std::string& str, std::size_t offset, std::size_t count) {
            const auto start = std::next(str.cbegin(), static_cast<std::string::difference_type>(offset));
            const auto end = std::next(start, static_cast<std::string::difference_type>(std::min(count, str.size() - offset)));
            std::copy(start, end, std::back_inserter(environment));
        };

        if (row != 0) {
            copy_row_part(data_[row - 1], start_scan_pos, max_scan_length);
        }

        if (col > 0) {
            environment.push_back(data_[row][col - 1]);
        }

        if ((col + number.size()) < data_[row].size()) {
            environment.push_back(data_[row][col + number.size()]);
        }

        if (row != (data_.size() - 1)) {
            copy_row_part(data_[row + 1], start_scan_pos, max_scan_length);
        }

        return std::any_of(environment.cbegin(), environment.cend(), [](char cell) {
            return !NOT_SYMBOLS.contains(cell);
        });
    }

private:
    std::vector<std::string> data_;
};


int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-3\input.txt)");

    const auto scheme = Scheme::load_scheme(document);
    const auto parts_numbers = scheme.get_parts_numbers();
    const auto result = std::reduce(parts_numbers.cbegin(), parts_numbers.cend());
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
