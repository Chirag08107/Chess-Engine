#include "../include/Queen.h"
#include "../include/Board.h"

Queen::Queen(Color c) : Piece(c)
{
}

PieceType Queen::getType() const
{
  return PieceType::QUEEN;
}

std::vector<Move> Queen::generateMoves(Board &board, int x, int y)
{
  std::vector<Move> moves;

  int directions[8][2] = {
      {1, 0}, {-1, 0}, {0, 1}, {0, -1}, // Rook directions
      {1, 1},
      {1, -1},
      {-1, 1},
      {-1, -1} // Bishop directions
  };

  for (int d = 0; d < 8; d++)
  {
    int dx = directions[d][0];
    int dy = directions[d][1];

    int newX = x + dx;
    int newY = y + dy;

    while (newX >= 0 && newX < 8 && newY >= 0 && newY < 8)
    {
      Piece *target = board.getPiece(newX, newY);

      if (target == nullptr)
      {
        moves.push_back(Move(x, y, newX, newY));
      }
      else
      {
        if (target->getColor() != color)
        {
          moves.push_back(Move(x, y, newX, newY));
        }
        break;
      }

      newX += dx;
      newY += dy;
    }
  }

  return moves;
}