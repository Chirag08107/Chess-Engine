#include "../include/King.h"
#include "../include/Board.h"

King::King(Color c) : Piece(c)
{
}

PieceType King::getType() const
{
  return PieceType::KING;
}

std::vector<Move> King::generateMoves(Board &board, int x, int y)
{
  std::vector<Move> moves;

  int directions[8][2] = {
      {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  for (int d = 0; d < 8; d++)
  {
    int newX = x + directions[d][0];
    int newY = y + directions[d][1];

    if (newX >= 0 && newX < 8 &&
        newY >= 0 && newY < 8)
    {
      Piece *target = board.getPiece(newX, newY);

      if (target == nullptr ||
          target->getColor() != color)
      {
        moves.push_back(Move(x, y, newX, newY));
      }
    }
  }

  return moves;
}