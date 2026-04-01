#include "../include/Knight.h"
#include "../include/Board.h"

Knight::Knight(Color c) : Piece(c)
{
}

PieceType Knight::getType() const
{
  return PieceType::KNIGHT;
}

std::vector<Move> Knight::generateMoves(Board &board, int x, int y)
{
  std::vector<Move> moves;

  int offsets[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

  for (int i = 0; i < 8; i++)
  {
    int newX = x + offsets[i][0];
    int newY = y + offsets[i][1];

    if (newX >= 0 && newX < 8 &&
        newY >= 0 && newY < 8)
    {
      Piece *target = board.getPiece(newX, newY);

      if (target == nullptr)
      {
        moves.push_back(Move(x, y, newX, newY));
      }
      else if (target->getColor() != color)
      {
        moves.push_back(Move(x, y, newX, newY));
      }
    }
  }

  return moves;
}