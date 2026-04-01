#include "../include/Rook.h"
#include "../include/Board.h"

Rook::Rook(Color c) : Piece(c) {}

PieceType Rook::getType() const
{
  return PieceType::ROOK;
}

std::vector<Move> Rook::generateMoves(Board &board, int x, int y)
{
  std::vector<Move> moves;

  int directions[4][2] = {
      {1, 0},  // right
      {-1, 0}, // left
      {0, 1},  // down
      {0, -1}  // up
  };

  for (int d = 0; d < 4; d++)
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