#pragma once
#include "Piece.h"
#include "Move.h"
#include <vector>

class Board
{
private:
  Piece *grid[8][8];

public:
  Board();
  ~Board();

  void initialize();

  Piece *getPiece(int x, int y);
  void setPiece(int x, int y, Piece *piece);

  void movePiece(const Move &move);

  // NEW FUNCTIONS
  bool isKingInCheck(Color color);
  std::vector<Move> getAllMoves(Color color);
};