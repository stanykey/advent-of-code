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

struct NodeInfo {
    std::string name;
    std::string left_name;
    std::string right_name;
};

NodeInfo parse_node_data(const std::string& data) {
    static const std::regex pattern(R"((\w+)\s*=\s*\((\w+),\s*(\w+)\))");

    std::smatch matches;
    if (!std::regex_match(data, matches, pattern)) {
        throw std::invalid_argument("Invalid input steam");
    }

    return {.name = matches[1], .left_name = matches[2], .right_name = matches[3]};
}

class Network {
public:
    struct Node {
        std::string name;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    const std::vector<std::unique_ptr<Node>>& nodes() const {
        return nodes_;
    }

    const std::unordered_map<std::string, Node*>& map() const {
        return tree_;
    }

public:
    friend std::istream& operator>>(std::istream& in, Network& network) {
        std::string data;
        std::vector<NodeInfo> nodes_info;
        while (std::getline(in, data) && !data.empty()) {
            nodes_info.emplace_back(parse_node_data(data));
            network.nodes_.emplace_back(new Node{.name = nodes_info.back().name});
            network.tree_[nodes_info.back().name] = network.nodes_.back().get();
        }

        for (const auto& info : nodes_info) {
            auto* node = network.tree_[info.name];
            node->left = network.tree_[info.left_name];
            node->right = network.tree_[info.right_name];
        }

        return in;
    }

private:
    std::vector<std::unique_ptr<Node>> nodes_;
    std::unordered_map<std::string, Node*> tree_;
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
    std::getline(document, raw_route);  // consume new line

    return route;
}

std::size_t calc_distance(
    const Network& network, const std::string& from, const std::string& to, const std::vector<std::uint16_t>& route
) {
    std::size_t distance = 0;

    auto current = from;
    std::size_t route_idx = 0;
    while (current != to) {
        const auto* node = network.map().at(current);

        current = route[route_idx] == 0 ? node->left->name : node->right->name;
        route_idx = (route_idx + 1) % route.size();

        distance++;
    }
    return distance;
}

using NodePredicate = std::function<bool(const Network::Node& node)>;

std::size_t
calc_distance(const Network& network, const NodePredicate& from, const NodePredicate& to, const std::vector<std::uint16_t>& route) {
    std::vector<const Network::Node*> currents;
    for (const auto& node : network.nodes()) {
        if (from(*node)) {
            currents.push_back(node.get());
        }
    }

    const auto is_done = [&currents, to]() {
        return std::all_of(currents.cbegin(), currents.cend(), [to](const Network::Node* node) { return to(*node); });
    };

    std::size_t distance = 0;
    std::size_t route_idx = 0;
    while (!is_done()) {
        for (auto& node : currents) {
            node = route[route_idx] == 0 ? node->left : node->right;
        }
        route_idx = (route_idx + 1) % route.size();
        distance++;
    }
    return distance;
}

int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-8\input.txt)");

    const auto route = load_route(document);
    const auto network = io::read<Network>(document);
    const auto distance = calc_distance(
        network, [](const Network::Node& node) -> bool { return node.name.ends_with('A'); },
        [](const Network::Node& node) -> bool { return node.name.ends_with('Z'); }, route
    );
    std::cout << "The result is " << distance << std::endl;
    return 0;
}
