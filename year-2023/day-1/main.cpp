#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>


std::vector<std::uint16_t> parse_digits(const std::string& message) {
    static constexpr auto DIGITS = R"(0123456789)";

    auto digit_pos = message.find_first_of(DIGITS);
    if (digit_pos == std::string::npos) {
        return {};
    }

    std::vector<std::uint16_t> digits;
    while (digit_pos != std::string::npos) {
        digits.emplace_back(message[digit_pos] - '0');
        digit_pos = message.find_first_of(DIGITS, digit_pos + 1);
    }
    return digits;
}

std::istringstream get_test_data() {
    return std::istringstream("1abc2\n"
                              "pqr3stu8vwx\n"
                              "a1b2c3d4e5f\n"
                              "treb7uchet\n");
}

std::unique_ptr<std::istream> get_document(const char* filename) {
    if (!filename) {
        return std::make_unique<std::istringstream>(get_test_data());
    }

    return std::make_unique<std::ifstream>(filename);
}

std::vector<std::uint16_t> get_calibrations(std::istream& in) {
    std::vector<std::uint16_t> calibrations;
    std::transform(
        std::istream_iterator<std::string>(in), std::istream_iterator<std::string>(), std::back_inserter(calibrations),
        [](const std::string& line) {
            const auto digits = parse_digits(line);
            if (digits.empty()) {
                return 0;
            }

            return digits.front() * 10 + digits.back();
        }
    );
    return calibrations;
}

int main(int argc, const char* argv[]) {
    const auto* filename = (argc == 2) ? argv[1] : nullptr;
    auto        document = get_document(filename);

    const std::vector<std::uint16_t> calibrations = get_calibrations(*document);
    std::cout << "calibration data (" << calibrations.size() << " items): ";
    std::copy(calibrations.cbegin(), calibrations.cend(), std::ostream_iterator<std::uint16_t>(std::cout, " "));
    std::cout << std::endl;

    const auto result = std::accumulate(calibrations.begin(), calibrations.end(), std::uint32_t{0});
    std::cout << "result value is " << result << std::endl;

    return 0;
}
