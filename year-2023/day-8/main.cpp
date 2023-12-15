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

struct Node {
    std::string name;
    std::string left;
    std::string right;
};

Node parse_node_data(const std::string& data) {
    static const std::regex pattern(R"((\w+)\s*=\s*\((\w+),\s*(\w+)\))");

    std::smatch matches;
    if (!std::regex_match(data, matches, pattern)) {
        throw std::invalid_argument("Invalid input steam");
    }

    return {.name = matches[1], .left = matches[2], .right = matches[3]};
}

class Network {
public:
    friend std::istream& operator>>(std::istream& in, Network& network) {
        std::string data;
        while (std::getline(in, data) && !data.empty()) {
            auto node = parse_node_data(data);
            network.nodes[node.name] = std::move(node);
        }
        return in;
    }

    std::unordered_map<std::string, Node> nodes;
};

std::vector<std::uint16_t> load_route(std::istream& document) {
    std::string raw_route;
    if (!std::getline(document, raw_route)) {
        throw std::invalid_argument("Invalid input stream");
    }

    std::vector<std::uint16_t> route(raw_route.size(), 0);
    std::transform(raw_route.cbegin(), raw_route.cend(), route.begin(), [](char direction) {
        return direction == 'L' ? 0 : 1;
    });
    std::getline(document, raw_route); // consume new line

    return route;
}

std::size_t calc_distance(
    const Network& network, const std::string& from, const std::string& to, const std::vector<std::uint16_t>& route
) {
    std::size_t distance = 0;

    auto current = from;
    std::size_t route_idx = 0;
    while (current != to) {
        const auto& node = network.nodes.at(current);

        current = route[route_idx] == 0 ? node.left : node.right;
        route_idx = (route_idx + 1) % route.size();

        distance++;
    }
    return distance;
}

int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-8\input.txt)");

    const auto route = load_route(document);
    const auto network = io::read<Network>(document);
    const auto distance = calc_distance(network, "AAA", "ZZZ", route);
    std::cout << "The result is " << distance << std::endl;
    return 0;
}
