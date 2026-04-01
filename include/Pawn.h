#pragma once
#include "Piece.h"

class Pawn : public Piece
{
public:
  Pawn(Color c);

  std::vector<Move> generateMoves(Board &board, int x, int y) override;

  PieceType getType() const override;
};