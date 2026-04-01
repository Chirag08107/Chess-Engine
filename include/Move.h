// define the piece moves from X(initial) and Y(initial) to final
#pragma once

class Move
{
public:
  int fromX;
  int fromY;
  int toX;
  int toY;

  // constructor
  Move(int fx, int fy, int tx, int ty)
      : fromX(fx), fromY(fy), toX(tx), toY(ty) {}
};