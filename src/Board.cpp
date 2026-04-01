#include "../include/Board.h"
#include "../include/Pawn.h"
#include "../include/Rook.h"
#include "../include/Knight.h"
#include "../include/Bishop.h"
#include "../include/Queen.h"
#include "../include/King.h"
#include <iostream>

Board::Board()
{
  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++)
      grid[x][y] = nullptr;
}

Board::~Board()
{
  for (int x = 0; x < 8; x++)
    for (int y = 0; y < 8; y++)
      delete grid[x][y];
}

void Board::initialize()
{
  for (int i = 0; i < 8; i++)
  {
    grid[i][6] = new Pawn(Color::WHITE);
    grid[i][1] = new Pawn(Color::BLACK);
  }

  grid[0][7] = new Rook(Color::WHITE);
  grid[7][7] = new Rook(Color::WHITE);
  grid[0][0] = new Rook(Color::BLACK);
  grid[7][0] = new Rook(Color::BLACK);

  grid[1][7] = new Knight(Color::WHITE);
  grid[6][7] = new Knight(Color::WHITE);
  grid[1][0] = new Knight(Color::BLACK);
  grid[6][0] = new Knight(Color::BLACK);

  grid[2][7] = new Bishop(Color::WHITE);
  grid[5][7] = new Bishop(Color::WHITE);
  grid[2][0] = new Bishop(Color::BLACK);
  grid[5][0] = new Bishop(Color::BLACK);

  grid[3][7] = new Queen(Color::WHITE);
  grid[3][0] = new Queen(Color::BLACK);

  grid[4][7] = new King(Color::WHITE);
  grid[4][0] = new King(Color::BLACK);

  std::cout << "Board initialized\n";
}

Piece *Board::getPiece(int x, int y)
{
  return grid[x][y];
}

void Board::setPiece(int x, int y, Piece *piece)
{
  grid[x][y] = piece;
}

void Board::movePiece(const Move &move)
{
  Piece *moving = grid[move.fromX][move.fromY];
  Piece *target = grid[move.toX][move.toY];

  // ❌ Prevent king capture
  if (target != nullptr && target->getType() == PieceType::KING)
    return;

  if (target != nullptr)
    delete target;

  grid[move.toX][move.toY] = moving;
  grid[move.fromX][move.fromY] = nullptr;
}

// 🔥 Get all moves of a color
std::vector<Move> Board::getAllMoves(Color color)
{
  std::vector<Move> moves;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece = grid[x][y];

      if (piece != nullptr && piece->getColor() == color)
      {
        std::vector<Move> pm = piece->generateMoves(*this, x, y);

        for (const Move &m : pm)
          moves.push_back(m);
      }
    }
  }

  return moves;
}

// 🔥 Check detection
bool Board::isKingInCheck(Color color)
{
  int kingX = -1, kingY = -1;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *p = grid[x][y];

      if (p != nullptr &&
          p->getType() == PieceType::KING &&
          p->getColor() == color)
      {
        kingX = x;
        kingY = y;
      }
    }
  }

  Color opponent = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;

  std::vector<Move> moves = getAllMoves(opponent);

  for (const Move &m : moves)
  {
    if (m.toX == kingX && m.toY == kingY)
      return true;
  }

  return false;
}