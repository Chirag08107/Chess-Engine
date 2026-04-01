// defines the RULE that every piece MUST define how it moves
// whenever it is called correspondingly the user should define generateMoves and getType

#pragma once
#include <vector>
#include "Type.h"
#include "Move.h"

class Board; // forward declaration

class Piece
{
protected:
  Color color;

public:
  // constructor
  Piece(Color c) : color(c) {}

  // destructor
  virtual ~Piece() {}

  Color getColor() const { return color; }

  // polymorphism
  virtual std::vector<Move> generateMoves(Board &board, int x, int y) = 0;
  virtual PieceType getType() const = 0;
};