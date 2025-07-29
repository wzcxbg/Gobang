#include "board.h"
#include <iostream>

Board::Board() {
    board.resize(BOARD_SIZE, std::vector<StoneType>(BOARD_SIZE, EMPTY));
}

Board::~Board() {
    // Destructor
}

bool Board::placeStone(int row, int col, StoneType player) {
    if (isValidMove(row, col)) {
        board[row][col] = player;
        return true;
    }
    return false;
}

bool Board::isValidMove(int row, int col) const {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && board[row][col] == EMPTY;
}

bool Board::checkWin(int row, int col, StoneType player) const {
    // Check horizontal
    if (checkLine(row, col, 0, 1, player)) return true;
    // Check vertical
    if (checkLine(row, col, 1, 0, player)) return true;
    // Check diagonal (top-left to bottom-right)
    if (checkLine(row, col, 1, 1, player)) return true;
    // Check anti-diagonal (top-right to bottom-left)
    if (checkLine(row, col, 1, -1, player)) return true;
    return false;
}

bool Board::checkLine(int r, int c, int dr, int dc, StoneType player) const {
    int count = 0;
    // Check in one direction
    for (int i = 0; i < 5; ++i) {
        int curR = r + i * dr;
        int curC = c + i * dc;
        if (curR >= 0 && curR < BOARD_SIZE && curC >= 0 && curC < BOARD_SIZE && board[curR][curC] == player) {
            count++;
        } else {
            break;
        }
    }
    // Check in opposite direction (excluding the starting stone)
    for (int i = 1; i < 5; ++i) {
        int curR = r - i * dr;
        int curC = c - i * dc;
        if (curR >= 0 && curR < BOARD_SIZE && curC >= 0 && curC < BOARD_SIZE && board[curR][curC] == player) {
            count++;
        } else {
            break;
        }
    }
    return count >= 5;
}

void Board::reset() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = EMPTY;
        }
    }
}

StoneType Board::getStone(int row, int col) const {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        return board[row][col];
    }
    return EMPTY; // Or throw an exception for invalid access
}
