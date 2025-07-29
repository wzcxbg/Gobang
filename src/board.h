#ifndef GOBANG_BOARD_H
#define GOBANG_BOARD_H

#include <vector>

// Define board dimensions
const int BOARD_SIZE = 15;

// Define stone types
enum StoneType {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

class Board {
public:
    Board();
    ~Board();

    // Place a stone on the board
    bool placeStone(int row, int col, StoneType player);

    // Check if a move is valid (within bounds and empty)
    bool isValidMove(int row, int col) const;

    // Check if the last placed stone results in a win
    bool checkWin(int row, int col, StoneType player) const;

    // Reset the board to empty state
    void reset();

    // Get the stone type at a specific position
    StoneType getStone(int row, int col) const;

private:
    std::vector<std::vector<StoneType>> board;

    // Helper function to check a line for 5 consecutive stones
    bool checkLine(int r, int c, int dr, int dc, StoneType player) const;
};

#endif //GOBANG_BOARD_H
