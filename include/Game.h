#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "Board.h"

#include <map>
#include <string>
#include <vector>

class Game
{
private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  Board board;

  int tileSize = 100;

  int selectedX = -1;
  int selectedY = -1;

  Color currentTurn = Color::WHITE;

  std::vector<Move> currentMoves;

  std::map<std::string, SDL_Texture *> textures;

  TTF_Font *font = nullptr;

  bool gameOver = false;
  bool aipending = false;
  bool running = true;

  std::string gameOverMessage;

  Uint32 aiMoveTime = 0;

public:
  Game();
  ~Game();

  void run();

  void handleEvents();
  void render();

  void drawBoard();
  void drawPieces();
  void drawHighlights();
  void drawCheckHighlight();

  void loadTextures();

  void makeAIMove();

  int evaluateBoard();

  int minimax(
      int depth,
      int alpha,
      int beta,
      bool isMaximizing);

  bool isCheckmate(Color color);

private:
  void gameLoop();

  void drawText(
      const std::string &text,
      int x,
      int y);

  void drawFilledCircle(
      int cx,
      int cy,
      int radius,
      SDL_Color color);

  SDL_Texture *loadTexture(
      const std::string &path);
};

#endif