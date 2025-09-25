#ifndef WASM_SCOUT_LIB_GAME_H
#define WASM_SCOUT_LIB_GAME_H

#include <vector>
#include <memory>

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <map>
#include <array>

namespace scout
{

    enum class Player
    {
        ONE,
        TWO,
        NONE
    };

    inline Player opponent(Player p)
    {
        if (p == Player::ONE)
            return Player::TWO;
        if (p == Player::TWO)
            return Player::ONE;
        return Player::NONE;
    }

    // Forward declaration
    class GameState;

    class GameStateMoveValuesEstimator
    {
    public:
        std::vector<float> estimateMoveValues(const GameState &state) const;
    };

    class GameState
    {
    public:
        // Public constants
        static constexpr int NUM_MOVES = 9;
        static constexpr int NUM_FEATURES = 47;
        static constexpr int SPECIAL_NOT_SET = -1;

        // Default constructor
        GameState();

        // Sparse initialization constructor
        GameState(Player currentPlayer, const std::map<int, int> &nonZeroValues,
                  int scoreOne, int scoreTwo, int specialOne, int specialTwo);

        GameState(Player currentPlayer, int scoreOne, int scoreTwo,
                  int specialOne, int specialTwo, std::array<int, 18> cells);

        // Game logic
        std::unique_ptr<GameState> move(int move) const;

        bool isMoveAllowed(int move) const;
        bool isGameOver() const { return _isGameOver; }
        Player getCurrentPlayer() const { return _currentPlayer; }
        std::optional<Player> getWinner() const { return _winner; }
        std::vector<float> encode() const;
        int getScoreOne() const { return _scoreOne; }
        int getScoreTwo() const { return _scoreTwo; }
        std::string toString() const;
        std::vector<int> getCells() const;

    private:
        Player _currentPlayer;
        bool _isGameOver;
        std::optional<Player> _winner;
        int _scoreOne;
        int _scoreTwo;
        int _specialOne;
        int _specialTwo;
        std::array<int, 18> _cells;

        int moveByCell(int cell) const;
        int nextCell(int cell) const;
        bool checkGameOver() const;
        std::optional<Player> checkWinner() const;
        Player isSpecial(int cell) const;
        bool isReachable(int cell) const;
        int boardCell(int move) const;
    };

    /**
     * @brief A simple class to track game outcomes (wins for each player and ties).
     */
    class Outcomes
    {
    public:
        Outcomes() = default;

        /**
         * @brief Calculates the win rate for a given player, where a tie counts as half a win.
         * @param player The player for whom to calculate the win rate.
         * @return A float representing the win rate [0.0, 1.0].
         */
        float winRateFor(Player player) const;

        // Increments the counter for the winning player (or ties).
        void addWinner(Player winner);

        // Returns the total number of games played.
        int getTotalOutcomes() const;

        // Creates a string representation of the object.
        std::string toString() const;

        // Overloads the == operator for equality checks.
        bool operator==(const Outcomes &other) const;

    private:
        int _first = 0;  // Wins for Player ONE
        int _second = 0; // Wins for Player TWO
        int _ties = 0;
    };
}

#endif // WASM_SCOUT_LIB_GAME_H