#pragma once
#include "Piece.h"

class King : public Piece
{
public:
  King(Color c);

  std::vector<Move> generateMoves(Board &board, int x, int y) override;
  PieceType getType() const override;
};