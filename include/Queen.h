#pragma once
#include "Piece.h"

class Queen : public Piece
{
public:
  Queen(Color c);

  std::vector<Move> generateMoves(Board &board, int x, int y) override;
  PieceType getType() const override;
};