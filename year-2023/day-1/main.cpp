#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <numeric>
#include <memory>


std::istringstream get_test_data() {
    return std::istringstream(
        "1abc2\n"
        "pqr3stu8vwx\n"
        "a1b2c3d4e5f\n"
        "treb7uchet\n"
    );
}

std::unique_ptr<std::istream> get_document(const char* filename) {
    if (!filename) {
        return std::make_unique<std::istringstream>(get_test_data());
    }

    return std::make_unique<std::ifstream>(filename);
}

int main(int argc, const char* argv[]) {
    const auto* filename = (argc == 2) ? argv[1] : nullptr;
    auto document = get_document(filename);
    std::vector<std::uint16_t> calibrations;
        std::transform(
            std::istream_iterator<std::string>(*document),
            std::istream_iterator<std::string>(),
            std::back_inserter(calibrations),
            [](const std::string& line) {
                static constexpr auto digits = R"(0123456789)";
                const auto first = line.find_first_of(digits);
                const auto second = line.find_last_of(digits);

                const auto number = line.substr(first, 1) + line.substr(second, 1);
                return std::stoul(number);
            }
        );

    std::cout << "calibration data (" << calibrations.size() << " items): ";
    std::copy(calibrations.cbegin(), calibrations.cend(), std::ostream_iterator<std::uint16_t>(std::cout, " "));
    std::cout << std::endl;

    const auto result = std::accumulate(calibrations.begin(), calibrations.end(), std::uint32_t{0});
    std::cout << "result value is " << result << std::endl;

    return 0;
}
