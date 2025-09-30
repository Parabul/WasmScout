#include "lib/game.h"

#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

using namespace emscripten;
#endif

namespace scout
{
    GameState::GameState()
        : _current_player(Player::ONE),
          _is_game_over(false),
          _winner(std::nullopt),
          _score_one(0),
          _score_two(0),
          _special_one(SPECIAL_NOT_SET),
          _special_two(SPECIAL_NOT_SET)
    {
        _cells.fill(9);
    }

    GameState::GameState(Player currentPlayer, int scoreOne, int scoreTwo,
                         int specialOne, int specialTwo, std::array<int, 18> cells)
        : _current_player(currentPlayer),
          _score_one(scoreOne),
          _score_two(scoreTwo),
          _special_one(specialOne),
          _special_two(specialTwo),
          _cells(cells)
    {
        _is_game_over = checkGameOver();
        _winner = checkWinner();
    }

    GameState::GameState(Player currentPlayer, const std::map<int, int> &nonZeroValues,
                         int scoreOne, int scoreTwo, int specialOne, int specialTwo)
        : _current_player(currentPlayer),
          _score_one(static_cast<int>(scoreOne)),
          _score_two(static_cast<int>(scoreTwo)),
          _special_one(static_cast<int>(specialOne)),
          _special_two(static_cast<int>(specialTwo))
    {
        _cells.fill(0);
        for (const auto &pair : nonZeroValues)
        {
            _cells[pair.first] = static_cast<int>(pair.second);
        }

        _is_game_over = checkGameOver();
        _winner = checkWinner();
    }

    std::vector<int> scout::GameState::getCells() const
    {
        return std::vector<int>(_cells.begin(), _cells.end());
    }

    // --- Private Helper Methods ---

    int GameState::moveByCell(int cell) const
    {
        if (cell < 9)
            return 8 - cell;
        return cell - 9;
    }

    int GameState::nextCell(int cell) const
    {
        if (cell == 0)
            return 9;
        if (cell < 9)
            return cell - 1;
        if (cell == 17)
            return 8;
        return cell + 1;
    }

    Player GameState::isSpecial(int cell) const
    {
        if (_special_one == cell)
            return Player::ONE;
        if (_special_two == cell)
            return Player::TWO;
        return Player::NONE;
    }

    bool GameState::isReachable(int cell) const
    {
        return (_current_player == Player::ONE) ? (cell > 8) : (cell < 9);
    }

    int GameState::boardCell(int move) const
    {
        if (_current_player == Player::ONE)
            return 8 - move;
        return 9 + move;
    }

    bool GameState::isMoveAllowed(int move) const
    {
        return _cells[boardCell(move)] != 0;
    }

    bool GameState::checkGameOver() const
    {
        if (_score_one > 81 || _score_two > 81)
            return true;
        if (_score_one == 81 && _score_two == 81)
            return true;

        // Equivalent to IntStream.range(0, 9).noneMatch(move -> isMoveAllowed(move));
        for (int move = 0; move < 9; ++move)
        {
            if (isMoveAllowed(move))
            {
                return false; // Found an allowed move, so game is not over
            }
        }
        return true; // No moves are allowed
    }

    std::optional<Player> GameState::checkWinner() const
    {
        if (!isGameOver())
            return std::nullopt;
        if (_score_one > 81)
            return Player::ONE;
        if (_score_two > 81)
            return Player::TWO;
        if (_score_one == 81 && _score_two == 81)
            return Player::NONE; // Draw

        bool noMovesAllowed = true;
        for (int move = 0; move < 9; ++move)
        {
            if (isMoveAllowed(move))
            {
                noMovesAllowed = false;
                break;
            }
        }
        if (noMovesAllowed)
        {
            return opponent(_current_player);
        }

        std::cout << toString() << "\n Unknown winner state";
        std::abort();
    }

    // --- Public Game Logic ---

    std::unique_ptr<GameState> GameState::move(int move) const
    {
        if (!isMoveAllowed(move))
        {
            std::cout << toString() << "\n The move is not allowed: " << move;
            std::abort();
        }

        int cell = boardCell(move);
        std::array<int, 18> newCells = _cells; // Copy cells
        int newScoreOne = _score_one;
        int newScoreTwo = _score_two;
        int newSpecialOne = _special_one;
        int newSpecialTwo = _special_two;

        int hand = newCells[cell];
        newCells[cell] = 0;

        // Rule A
        int currentCell = (hand == 1) ? nextCell(cell) : cell;

        while (hand > 0)
        {
            hand--;
            Player special = isSpecial(currentCell);

            // Rule C
            if (special == Player::NONE)
            {
                newCells[currentCell]++;
            }
            else
            {
                if (special == Player::ONE)
                    newScoreOne++;
                if (special == Player::TWO)
                    newScoreTwo++;
            }

            // Rule B
            if (hand == 0 && isReachable(currentCell))
            {
                if (newCells[currentCell] % 2 == 0)
                {
                    if (_current_player == Player::ONE)
                        newScoreOne += newCells[currentCell];
                    if (_current_player == Player::TWO)
                        newScoreTwo += newCells[currentCell];
                    newCells[currentCell] = 0;
                }

                // Rule D
                if (newCells[currentCell] == 3)
                {
                    int possibleSpecialCellMove = moveByCell(currentCell);
                    bool canSetSpecial = (possibleSpecialCellMove != 8);

                    if (_current_player == Player::ONE && newSpecialOne == SPECIAL_NOT_SET && canSetSpecial &&
                        (newSpecialTwo == SPECIAL_NOT_SET || possibleSpecialCellMove != moveByCell(newSpecialTwo)))
                    {
                        newScoreOne += 3;
                        newCells[currentCell] = 0;
                        newSpecialOne = static_cast<int>(currentCell);
                    }

                    if (_current_player == Player::TWO && newSpecialTwo == SPECIAL_NOT_SET && canSetSpecial &&
                        (newSpecialOne == SPECIAL_NOT_SET || possibleSpecialCellMove != moveByCell(newSpecialOne)))
                    {
                        newScoreTwo += 3;
                        newCells[currentCell] = 0;
                        newSpecialTwo = static_cast<int>(currentCell);
                    }
                }
            }
            currentCell = nextCell(currentCell);
        }

        auto new_state = std::make_unique<GameState>(opponent(_current_player), newScoreOne, newScoreTwo, newSpecialOne, newSpecialTwo, newCells);

        return new_state;
    }

    std::string GameState::toString() const
    {
        std::stringstream ss;
        ss << "-------------------------------\n";
        ss << static_cast<int>(_score_one) << ":" << static_cast<int>(_score_two) << "\n";

        auto print_row = [&](int start_index)
        {
            ss << "|";
            for (int i = start_index; i < start_index + 9; ++i)
            {
                std::stringstream cell_ss;
                cell_ss << static_cast<int>(_cells[i]);
                if (isSpecial(i) != Player::NONE)
                {
                    cell_ss << "*";
                }
                // Simulates Guava's Strings.padStart
                ss << std::setw(4) << std::right << cell_ss.str() << "|";
            }
            ss << "\n";
        };

        print_row(0);
        print_row(9);

        ss << "Current Player: " << (_current_player == Player::ONE ? "ONE" : "TWO") << "\n";
        ss << "Is GameOver: " << (_is_game_over ? "true" : "false") << "\n";
        ss << "Winner: ";
        if (_winner.has_value())
        {
            if (_winner.value() == Player::ONE)
                ss << "ONE";
            else if (_winner.value() == Player::TWO)
                ss << "TWO";
            else
                ss << "NONE";
        }
        else
        {
            ss << "null";
        }
        ss << "\n";

        return ss.str();
    }

    std::vector<float> GameState::encode() const
    {
        std::vector<float> encoded(NUM_FEATURES, 0.0f);

        if (_current_player == Player::ONE)
        {
            if (_special_one != SPECIAL_NOT_SET)
                encoded[moveByCell(_special_one)] = 1.0f;
            if (_special_two != SPECIAL_NOT_SET)
                encoded[9 + moveByCell(_special_two)] = 1.0f;
            for (int i = 0; i < 9; ++i)
                encoded[18 + i] = static_cast<float>(_cells[8 - i]) / 81.0f;
            for (int i = 0; i < 9; ++i)
                encoded[27 + i] = static_cast<float>(_cells[9 + i]) / 81.0f;
            encoded[36] = static_cast<float>(_score_one) / 81.0f;
            encoded[37] = static_cast<float>(_score_two) / 81.0f;
        }
        else
        { // Player TWO
            if (_special_two != SPECIAL_NOT_SET)
                encoded[moveByCell(_special_two)] = 1.0f;
            if (_special_one != SPECIAL_NOT_SET)
                encoded[9 + moveByCell(_special_one)] = 1.0f;
            for (int i = 0; i < 9; ++i)
                encoded[18 + i] = static_cast<float>(_cells[9 + i]) / 81.0f;
            for (int i = 0; i < 9; ++i)
                encoded[27 + i] = static_cast<float>(_cells[8 - i]) / 81.0f;
            encoded[36] = static_cast<float>(_score_two) / 81.0f;
            encoded[37] = static_cast<float>(_score_one) / 81.0f;
        }

        GameStateMoveValuesEstimator estimator;
        std::vector<float> moveValues = estimator.estimateMoveValues(*this);
        std::copy(moveValues.begin(), moveValues.end(), encoded.begin() + 38);

        return encoded;
    }

    std::vector<float> GameStateMoveValuesEstimator::estimateMoveValues(const GameState &state) const
    {
        // Create a vector to hold the estimated value of each move.
        std::vector<float> values(GameState::NUM_MOVES, 0.0f);

        // Calculate the score difference from the current player's perspective.
        float parentDiff = (state.getCurrentPlayer() == Player::ONE)
                               ? static_cast<float>(state.getScoreOne() - state.getScoreTwo())
                               : static_cast<float>(state.getScoreTwo() - state.getScoreOne());

        // Iterate through all possible moves to evaluate them.
        for (int i = 0; i < GameState::NUM_MOVES; ++i)
        {
            // If the move isn't allowed, its value is 0; skip to the next move.
            if (!state.isMoveAllowed(i))
            {
                continue;
            }

            // Simulate the move to get the resulting "child" game state.
            auto childState = state.move(i);

            // Calculate the score difference in the new state, still from the original player's perspective.
            float childDiff = (state.getCurrentPlayer() == Player::ONE)
                                  ? static_cast<float>(childState->getScoreOne() - childState->getScoreTwo())
                                  : static_cast<float>(childState->getScoreTwo() - childState->getScoreOne());

            // The value of the move is the normalized change in score difference.
            // This rewards moves that increase the player's score advantage.
            values[i] = (childDiff - parentDiff) / 81.0f;
        }

        return values;
    }

    float Outcomes::winRateFor(Player player) const
    {
        int total = getTotalOutcomes();
        if (total == 0)
        {
            return 0.0f;
        }

        switch (player)
        {
        case Player::ONE:
            return (1.0f * _first + 0.5f * _ties) / total;
        case Player::TWO:
            return (1.0f * _second + 0.5f * _ties) / total;
        default:
            return 1.0f * _ties / total;
        }
    }

    void Outcomes::addWinner(Player winner)
    {
        switch (winner)
        {
        case Player::ONE:
            _first++;
            break;
        case Player::TWO:
            _second++;
            break;
        default:
            _ties++;
            break;
        }
    }

    int Outcomes::getTotalOutcomes() const
    {
        return _first + _second + _ties;
    }

    std::string Outcomes::toString() const
    {
        std::stringstream ss;
        ss << "Outcomes{first=" << _first
           << ", second=" << _second
           << ", ties=" << _ties << "}";
        return ss.str();
    }

    bool Outcomes::operator==(const Outcomes &other) const
    {
        return this->_first == other._first &&
               this->_second == other._second &&
               this->_ties == other._ties;
    }

#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_BINDINGS(scout)
    {
        class_<GameState>("GameState")
            .constructor<>()
            .function("toString", &GameState::toString)
            .function("move", &GameState::move)
            .property("score_one", &GameState::getScoreOne)
            .property("score_two", &GameState::getScoreTwo)
            .property("special_one", &GameState::getSpecialOne)
            .property("special_two", &GameState::getSpecialTwo)
            .property("current_player", &GameState::getCurrentPlayer)
            .property("is_game_over", &GameState::isGameOver)
            .property("winner", &GameState::getWinner)
            .property("cells", &GameState::getCells);

        register_vector<int>("vector<int>");

        enum_<Player>("Player")
            .value("ONE", Player::ONE)
            .value("TWO", Player::TWO);

        register_optional<Player>();
    }

#endif

} // scout
