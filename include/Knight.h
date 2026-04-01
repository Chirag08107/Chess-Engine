#pragma once
#include "Piece.h"

class Knight : public Piece
{
public:
  Knight(Color c);

  std::vector<Move> generateMoves(Board &board, int x, int y) override;
  PieceType getType() const override;
};