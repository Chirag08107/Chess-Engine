// defines how will the pawn move

#include "../include/Pawn.h"
#include "../include/Board.h"

Pawn::Pawn(Color c) : Piece(c)
{
}

PieceType Pawn::getType() const
{
  return PieceType::PAWN;
}

std::vector<Move> Pawn::generateMoves(Board &board, int x, int y)
{
  std::vector<Move> moves;

  // if the pawn color is white then move up(-1) and if black then move down(+1)
  int direction = (color == Color::WHITE) ? -1 : 1;

  int oneStepY = y + direction;

  // boundary check
  if (oneStepY >= 0 && oneStepY < 8)
  {
    if (board.getPiece(x, oneStepY) == nullptr)
    {
      moves.push_back(Move(x, y, x, oneStepY));

      // Double move from start
      if ((color == Color::WHITE && y == 6) ||
          (color == Color::BLACK && y == 1))
      {
        int twoStepY = y + 2 * direction;

        if (twoStepY >= 0 && twoStepY < 8 &&
            board.getPiece(x, twoStepY) == nullptr)
        {
          moves.push_back(Move(x, y, x, twoStepY));
        }
      }
    }
  }

  // Diagonal captures
  int captureLeftX = x - 1;
  int captureRightX = x + 1;

  if (oneStepY >= 0 && oneStepY < 8)
  {
    // Capture Left
    if (captureLeftX >= 0)
    {
      Piece *target = board.getPiece(captureLeftX, oneStepY);

      if (target != nullptr && target->getColor() != color)
      {
        moves.push_back(Move(x, y, captureLeftX, oneStepY));
      }
    }

    // Capture Right
    if (captureRightX < 8)
    {
      Piece *target = board.getPiece(captureRightX, oneStepY);

      if (target != nullptr && target->getColor() != color)
      {
        moves.push_back(Move(x, y, captureRightX, oneStepY));
      }
    }
  }

  return moves;
}