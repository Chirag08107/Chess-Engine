#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Board.h"
#include <map>

class Game
{
private:
  sf::RenderWindow window;
  Board board;

  int tileSize = 100;

  int selectedX = -1;
  int selectedY = -1;

  Color currentTurn = Color::WHITE;

  std::vector<Move> currentMoves;

  std::map<std::string, sf::Texture> textures;

  // 🔥 GAME OVER UI
  sf::Font font;
  sf::Text gameOverText;
  bool gameOver = false;

public:
  Game();

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
  int minimax(int depth, int alpha, int beta, bool isMaximizing);

  bool aipending = false;
  bool isCheckmate(Color color);
};

#endif