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

    [[nodiscard]] std::uint32_t get_score() const {
        if (score_) {
            return score_.value();
        }

        std::vector<std::uint32_t> matches;
        std::set_intersection(
            winning_numbers_.cbegin(), winning_numbers_.cend(), draft_numbers_.cbegin(), draft_numbers_.cend(),
            std::back_inserter(matches)
        );

        return score_.emplace(1 << (matches.size() - 1));
    }

private:
    std::uint32_t id_;
    std::vector<std::uint32_t> winning_numbers_;
    std::vector<std::uint32_t> draft_numbers_;
    mutable std::optional<std::uint32_t> score_;
};

std::vector<Card> load_cards(std::istream& document) {
    std::vector<Card> cards;

    std::string record;
    while (std::getline(document, record)) {
        cards.emplace_back(Card::load_card(record));
    }

    return cards;
}


int main() {
    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-4\input.txt)");

    const auto cards = load_cards(document);
    const auto result =
        std::transform_reduce(cards.cbegin(), cards.cend(), std::uint32_t{0}, std::plus{}, [](const Card& card) {
            return card.get_score();
        });
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
