#pragma once
#include "Piece.h"

class Rook : public Piece
{
public:
  Rook(Color c);

  std::vector<Move> generateMoves(Board &board, int x, int y) override;
  PieceType getType() const override;
};