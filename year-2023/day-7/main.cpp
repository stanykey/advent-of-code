#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
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

namespace game {
    enum class Card { JOKER, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, QUEEN, KING, ACE };

    enum class Combination { ONE, PAIR, TWO_PAIR, THREE, FULL_HOUSE, FOUR, FIVE };

    constexpr std::size_t HAND_SIZE = 5;
    using Hand = std::array<Card, HAND_SIZE>;

    Hand read_hand(std::string_view hand_str) {
        Hand hand;
        std::transform(hand_str.cbegin(), hand_str.cend(), hand.begin(), [](char card) {
            static const std::unordered_map<char, Card> mapping{
                {'2', Card::TWO},   {'3', Card::THREE}, {'4', Card::FOUR}, {'5', Card::FIVE}, {'6', Card::SIX},
                {'7', Card::SEVEN}, {'8', Card::EIGHT}, {'9', Card::NINE}, {'T', Card::TEN},  {'J', Card::JOKER},
                {'Q', Card::QUEEN}, {'K', Card::KING},  {'A', Card::ACE}
            };
            return mapping.at(card);
        });
        return hand;
    }

    Combination determine_combination(const Hand& hand) {
        std::unordered_map<Card, std::size_t> set(hand.size());
        for (auto&& card : hand) {
            set[card]++;
        }

        const auto it = set.find(Card::JOKER);
        if (it != set.cend()) {
            const auto jokers = it->second;
            set.erase(it);
            if (set.empty()) {
                return Combination::FIVE;
            }

            auto max_it = std::max_element(set.begin(), set.end(), [](auto&& lhs, auto&& rhs) {
                return lhs.second < rhs.second;
            });
            (*max_it).second +=jokers;
        }

        if (set.size() == 1) {
            return Combination::FIVE;
        }

        if (set.size() == 2) {
            const auto marker = set.cbegin()->second;
            return (marker == 2 || marker == 3) ? Combination::FULL_HOUSE : Combination::FOUR;
        }

        if (set.size() == 3) {
            const auto it = std::find_if(set.cbegin(), set.cend(), [](auto&& entry) { return entry.second == 3; });
            return (it != set.cend()) ? Combination::THREE : Combination::TWO_PAIR;
        }

        return (set.size() == 4) ? Combination::PAIR : Combination::ONE;
    }

    struct Player {
        Player()
            : hand_()
            , bid_(0)
        {
        }

        const Hand& hand() const {
            return hand_;
        }

        std::size_t bid() const {
            return bid_;
        }

        Combination combo() const {
            if (combo_) {
                return combo_.value();
            }

            const auto value = determine_combination(hand_);
            return combo_.emplace(value);
        }

        friend std::istream& operator>>(std::istream& in, Player& player) {
            const auto hand = io::read<std::string>(in);
            player.hand_ = read_hand(hand);
            return in >> player.bid_;
            ;
        }

    private:
        Hand hand_;
        std::size_t bid_;
        mutable std::optional<Combination> combo_;
    };

}  // namespace game


std::vector<game::Player> load_players(std::istream& document) {
    std::vector<game::Player> players;
    std::copy(
        std::istream_iterator<game::Player>(document), std::istream_iterator<game::Player>(),
        std::back_inserter(players)
    );
    return players;
}

std::vector<game::Player>& sort_by_rank(std::vector<game::Player>& players) {
    std::sort(players.begin(), players.end(), [](const game::Player& lhs, const game::Player& rhs) {
        if (lhs.combo() == rhs.combo()) {
            return lhs.hand() < rhs.hand();
        }

        return lhs.combo() < rhs.combo();
    });
    return players;
}

int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-7\input.txt)");

    auto players = load_players(document);
    sort_by_rank(players);

    std::uint64_t result = 0;
    for (auto i = 0u; i != players.size(); i++) {
        result += players[i].bid() * (i + 1);
    }
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
