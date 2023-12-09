#include <algorithm>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>


template <typename Integer>
Integer as_int(std::string_view str) {
    Integer value{};
    const auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec == std::errc::invalid_argument || ec == std::errc::result_out_of_range) {
        throw std::invalid_argument("Invalid conversion to integer.");
    }
    return value;
}

struct Cubes {
    std::uint32_t red = 0;
    std::uint32_t green = 0;
    std::uint32_t blue = 0;

    bool operator<=(const Cubes& other) const {
        return red <= other.red && green <= other.green && blue <= other.blue;
    }
};

struct Bag {
    Cubes cubes;
};

struct Game {
    [[nodiscard]] Cubes maximized_set() const {
        std::uint32_t max_red = 0;
        std::uint32_t max_green = 0;
        std::uint32_t max_blue = 0;
        for (auto&& set : sets) {
            max_red = std::max(max_red, set.red);
            max_green = std::max(max_green, set.green);
            max_blue = std::max(max_blue, set.blue);
        }

        return Cubes{.red = max_red, .green = max_green, .blue = max_blue};
    }

    std::uint32_t id = 0;
    std::vector<Cubes> sets;
};

std::vector<Game> filter_games(const std::vector<Game>& games, const Bag& bag) {
    std::vector<Game> matches;

    matches.reserve(games.size());
    std::ranges::copy_if(games, std::back_inserter(matches), [&bag](const Game& game) {
        return game.maximized_set() <= bag.cubes;
    });
    matches.shrink_to_fit();

    return matches;
}

Game parse_game_record(std::string_view record) {
    // Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green
    static constexpr std::string_view GAME_PREFIX = "Game ";
    static constexpr std::string_view RED_COLOR = "red";
    static constexpr std::string_view GREEN_COLOR = "green";
    static constexpr std::string_view BLUE_COLOR = "blue";

    Game game;

    record.remove_prefix(GAME_PREFIX.size());
    const auto id_end = record.find(':');
    game.id = as_int<std::uint32_t>(record.substr(0, id_end));
    record.remove_prefix(id_end + 2); // eat ": "

    while (!record.empty()) {
        Cubes cubes;

        const auto set_delimiter = record.find(';');
        auto sets_info = record.substr(0, set_delimiter);
        while (!sets_info.empty()) {
            const auto delimiter = sets_info.find(',');
            const auto cubes_info = sets_info.substr(0, delimiter);

            const auto whitespace_pos = cubes_info.find(' ');
            const auto count = cubes_info.substr(0, whitespace_pos);
            const auto color = cubes_info.substr(whitespace_pos + 1);
            if (color == RED_COLOR) {
                cubes.red = as_int<std::int32_t>(count);
            } else if (color == GREEN_COLOR) {
                cubes.green = as_int<std::int32_t>(count);
            } else if (color == BLUE_COLOR) {
                cubes.blue = as_int<std::int32_t>(count);
            }

            const auto need_to_consume = delimiter != std::string_view::npos ? cubes_info.size() + 2 : cubes_info.size();
            sets_info.remove_prefix(need_to_consume);
            record.remove_prefix(need_to_consume);
        }
        record.remove_prefix(set_delimiter != std::string_view::npos ? 2 : 0); // consume "; "

        game.sets.emplace_back(cubes);
    }

    return game;
}

std::vector<Game> read_games(std::istream& game_records) {
    std::vector<Game> games;

    std::string record;
    while (std::getline(game_records, record)) {
        games.emplace_back(parse_game_record(record));
    }
    return games;
}

int main() {
    static const Bag BagConfiguration{{
        .red = 12,
        .green = 13,
        .blue = 14,
    }};

    std::ifstream document(R"(D:\work\advent-of-code\year-2023\day-2\input.txt)");
    const auto games = read_games(document);
    const auto result = std::transform_reduce(
        games.cbegin(), games.cend(), std::uint32_t{0}, std::plus{},
        [](const Game& game) {
            const auto maximized_set = game.maximized_set();
            return maximized_set.red * maximized_set.green * maximized_set.blue;
        }
    );
    std::cout << "The result value is " << result << std::endl;

    return 0;
}
