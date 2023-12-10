#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
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


struct Card {
public:
    static inline std::size_t WINNING_SET_SIZE = 5;
    static inline std::size_t DRAFT_SET_SIZE = 8;

    static Card load_card(const std::string& data) {
        std::istringstream in(data);

        Card card;
        std::ignore = io::read<std::string>(in);
        card.id_ = io::read<std::uint32_t>(in);
        in.ignore(2);  // consume ' : '

        std::copy(
            std::istream_iterator<std::uint32_t>(in), std::istream_iterator<std::uint32_t>(),
            std::back_inserter(card.winning_numbers_)
        );
        std::sort(card.winning_numbers_.begin(), card.winning_numbers_.end());

        in.clear();
        in.ignore(2);  // consume ' | '

        std::copy(
            std::istream_iterator<std::uint32_t>(in), std::istream_iterator<std::uint32_t>(),
            std::back_inserter(card.draft_numbers_)
        );
        std::sort(card.draft_numbers_.begin(), card.draft_numbers_.end());

        return card;
    }

    std::uint32_t id() const {
        return id_;
    }

    [[nodiscard]] std::tuple<std::uint32_t, const std::vector<std::uint32_t>&> get_score() const {
        if (score_) {
            return {score_.value(), matches_};
        }

        std::set_intersection(
            winning_numbers_.cbegin(), winning_numbers_.cend(), draft_numbers_.cbegin(), draft_numbers_.cend(),
            std::back_inserter(matches_)
        );

        const auto value = matches_.empty() ? 0 : (1 << (matches_.size() - 1));
        return {score_.emplace(value), matches_};
    }

private:
    std::uint32_t id_;
    std::vector<std::uint32_t> winning_numbers_;
    std::vector<std::uint32_t> draft_numbers_;
    mutable std::optional<std::uint32_t> score_;
    mutable std::vector<std::uint32_t> matches_;
};

std::vector<Card> load_cards(std::istream& document) {
    std::vector<Card> cards;

    std::string record;
    while (std::getline(document, record)) {
        cards.emplace_back(Card::load_card(record));
    }

    return cards;
}

std::uint32_t play_game(const std::vector<Card>& cards) {
    std::unordered_map<std::uint32_t, std::uint32_t> scores;
    std::unordered_map<std::uint32_t, std::uint32_t> counter;

    const auto copy_cards = [&](std::size_t id, std::size_t count) {
        while (id != cards.size() && count != 0) {
            counter[cards[id].id()]++;
            count--;
        }
    };

    for (const auto& card : cards) {
        auto&& [score, matches] = card.get_score();

        counter[card.id()]++;
        scores[card.id()] = score;

        copy_cards(card.id() + 1, matches.size());
    }

    return std::accumulate(
        counter.cbegin(), counter.cend(), std::uint32_t{0},
        [&](std::uint32_t value, const auto& entry) {
            const auto [card_id, cards_count] = entry;
            return value + scores[card_id] * counter[card_id];
        }
    );
}


int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-4\test-data.txt)");

    const auto cards = load_cards(document);
    const auto result = play_game(cards);
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
