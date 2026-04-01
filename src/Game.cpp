#include "../include/Game.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

Game::Game()
    : window(sf::VideoMode(800, 800), "Chess Engine")
{
  window.setFramerateLimit(60);

  board.initialize();
  loadTextures();

  srand(time(0));

  // 🔤 LOAD FONT
  if (!font.loadFromFile("../assets/Roboto.ttf"))
  {
    std::cout << "Error loading font\n";
  }

  gameOverText.setFont(font);
  gameOverText.setCharacterSize(50);
  gameOverText.setFillColor(sf::Color::Red);
  gameOverText.setPosition(200, 350);
}

void Game::run()
{
  while (window.isOpen())
  {
    handleEvents();

    // AI executes OUTSIDE event loop
    if (aipending && !gameOver)
    {
      sf::sleep(sf::milliseconds(400)); // delay here ✅

      makeAIMove();

      if (isCheckmate(Color::WHITE))
      {
        gameOver = true;
        gameOverText.setString("BLACK WINS!");
      }

      currentTurn = Color::WHITE;
      aipending = false;
    }

    render();
  }
}

void Game::handleEvents()
{
  sf::Event event;

  while (window.pollEvent(event))
  {
    if (event.type == sf::Event::Closed)
      window.close();

    // 🔥 STOP INPUT AFTER GAME OVER
    if (gameOver)
      continue;

    if (event.type == sf::Event::MouseButtonPressed)
    {
      int x = event.mouseButton.x / tileSize;
      int y = event.mouseButton.y / tileSize;

      // 🟢 SELECT PIECE
      if (selectedX == -1)
      {
        Piece *piece = board.getPiece(x, y);

        if (piece && piece->getColor() == currentTurn)
        {
          selectedX = x;
          selectedY = y;

          auto rawMoves = piece->generateMoves(board, x, y);
          currentMoves.clear();

          // 🔥 FILTER LEGAL MOVES
          for (const Move &m : rawMoves)
          {
            Piece *captured = board.getPiece(m.toX, m.toY);
            Piece *moving = board.getPiece(m.fromX, m.fromY);

            board.setPiece(m.toX, m.toY, moving);
            board.setPiece(m.fromX, m.fromY, nullptr);

            bool safe = !board.isKingInCheck(currentTurn);

            board.setPiece(m.fromX, m.fromY, moving);
            board.setPiece(m.toX, m.toY, captured);

            if (safe)
              currentMoves.push_back(m);
          }
        }
      }
      // 🔵 MOVE PIECE
      else
      {
        for (const Move &m : currentMoves)
        {
          if (m.toX == x && m.toY == y)
          {
            // PLAYER MOVE
            board.movePiece(m);

            // 🔥 CHECKMATE AFTER PLAYER MOVE
            if (isCheckmate(Color::BLACK))
            {
              gameOver = true;
              gameOverText.setString("WHITE WINS!");
              return;
            }

            aipending = true;
            currentTurn = Color::BLACK;

            // 🔥 CHECKMATE AFTER AI MOVE
            if (isCheckmate(Color::WHITE))
            {
              gameOver = true;
              gameOverText.setString("BLACK WINS!");
              return;
            }

            currentTurn = Color::WHITE;
            break;
          }
        }

        // RESET SELECTION
        selectedX = -1;
        selectedY = -1;
        currentMoves.clear();
      }
    }
  }
}

void Game::drawCheckHighlight()
{
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece = board.getPiece(x, y);

      if (piece && piece->getType() == PieceType::KING)
      {
        if ((piece->getColor() == Color::WHITE && board.isKingInCheck(Color::WHITE)) ||
            (piece->getColor() == Color::BLACK && board.isKingInCheck(Color::BLACK)))
        {
          sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
          highlight.setPosition(x * tileSize, y * tileSize);
          highlight.setFillColor(sf::Color(255, 0, 0, 100)); // RED overlay

          window.draw(highlight);
        }
      }
    }
  }
}

bool Game::isCheckmate(Color color)
{
  if (!board.isKingInCheck(color))
    return false;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece = board.getPiece(x, y);

      if (piece && piece->getColor() == color)
      {
        auto moves = piece->generateMoves(board, x, y);

        for (const Move &m : moves)
        {
          Piece *captured = board.getPiece(m.toX, m.toY);
          Piece *moving = board.getPiece(m.fromX, m.fromY);

          // simulate
          board.setPiece(m.toX, m.toY, moving);
          board.setPiece(m.fromX, m.fromY, nullptr);

          bool safe = !board.isKingInCheck(color);

          // undo
          board.setPiece(m.fromX, m.fromY, moving);
          board.setPiece(m.toX, m.toY, captured);

          if (safe)
            return false;
        }
      }
    }
  }

  return true;
}

int getPieceValue(PieceType type)
{
  switch (type)
  {
  case PieceType::PAWN:
    return 10;
  case PieceType::KNIGHT:
    return 30;
  case PieceType::BISHOP:
    return 30;
  case PieceType::ROOK:
    return 50;
  case PieceType::QUEEN:
    return 90;
  case PieceType::KING:
    return 1000;
  default:
    return 0;
  }
}

int Game::evaluateBoard()
{
  int score = 0;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece = board.getPiece(x, y);

      if (piece != nullptr)
      {
        int value = getPieceValue(piece->getType());

        if (piece->getColor() == Color::BLACK)
          score += value;
        else
          score -= value;
      }
    }
  }

  return score;
}

int Game::minimax(int depth, int alpha, int beta, bool isMaximizing)
{
  if (depth == 0)
    return evaluateBoard();

  if (isMaximizing) // AI (BLACK)
  {
    int best = -100000;

    for (int y = 0; y < 8; y++)
    {
      for (int x = 0; x < 8; x++)
      {
        Piece *piece = board.getPiece(x, y);

        if (piece && piece->getColor() == Color::BLACK)
        {
          auto moves = piece->generateMoves(board, x, y);

          for (const Move &m : moves)
          {
            Piece *captured = board.getPiece(m.toX, m.toY);
            Piece *moving = board.getPiece(m.fromX, m.fromY);

            board.setPiece(m.toX, m.toY, moving);
            board.setPiece(m.fromX, m.fromY, nullptr);

            if (!board.isKingInCheck(Color::BLACK))
            {
              int score = minimax(depth - 1, alpha, beta, false);
              best = std::max(best, score);
              alpha = std::max(alpha, best);
            }

            board.setPiece(m.fromX, m.fromY, moving);
            board.setPiece(m.toX, m.toY, captured);

            // 🔥 PRUNING
            if (beta <= alpha)
              break;
          }
        }
      }
    }

    return best;
  }
  else // Player (WHITE)
  {
    int best = 100000;

    for (int y = 0; y < 8; y++)
    {
      for (int x = 0; x < 8; x++)
      {
        Piece *piece = board.getPiece(x, y);

        if (piece && piece->getColor() == Color::WHITE)
        {
          auto moves = piece->generateMoves(board, x, y);

          for (const Move &m : moves)
          {
            Piece *captured = board.getPiece(m.toX, m.toY);
            Piece *moving = board.getPiece(m.fromX, m.fromY);

            board.setPiece(m.toX, m.toY, moving);
            board.setPiece(m.fromX, m.fromY, nullptr);

            if (!board.isKingInCheck(Color::WHITE))
            {
              int score = minimax(depth - 1, alpha, beta, true);
              best = std::min(best, score);
              beta = std::min(beta, best);
            }

            board.setPiece(m.fromX, m.fromY, moving);
            board.setPiece(m.toX, m.toY, captured);

            // 🔥 PRUNING
            if (beta <= alpha)
              break;
          }
        }
      }
    }

    return best;
  }
}

void Game::makeAIMove()
{
  int bestScore = -100000;
  Move bestMove(0, 0, 0, 0);

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece = board.getPiece(x, y);

      if (piece && piece->getColor() == Color::BLACK)
      {
        auto moves = piece->generateMoves(board, x, y);

        for (const Move &m : moves)
        {
          Piece *captured = board.getPiece(m.toX, m.toY);
          Piece *moving = board.getPiece(m.fromX, m.fromY);

          // simulate move
          board.setPiece(m.toX, m.toY, moving);
          board.setPiece(m.fromX, m.fromY, nullptr);

          if (!board.isKingInCheck(Color::BLACK))
          {
            int score = minimax(2, -100000, 100000, false);

            if (score > bestScore)
            {
              bestScore = score;
              bestMove = m;
            }
          }

          // undo move
          board.setPiece(m.fromX, m.fromY, moving);
          board.setPiece(m.toX, m.toY, captured);
        }
      }
    }
  }

  board.movePiece(bestMove);
}

void Game::render()
{
  window.clear();
  drawBoard();
  drawCheckHighlight();
  drawHighlights();
  drawPieces();

  if (gameOver)
  {
    window.draw(gameOverText);
  }

  window.display();
}

void Game::drawBoard()
{
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      sf::RectangleShape square(sf::Vector2f(tileSize, tileSize));
      square.setPosition(x * tileSize, y * tileSize);

      if ((x + y) % 2 == 0)
        square.setFillColor(sf::Color(240, 217, 181));
      else
        square.setFillColor(sf::Color(181, 136, 99));

      window.draw(square);
    }
  }
}

void Game::loadTextures()
{
  // 🔥 IMAGES LOADED HERE
  textures["white_pawn"].loadFromFile("../assets/wP.png");
  textures["black_pawn"].loadFromFile("../assets/bP.png");

  textures["white_rook"].loadFromFile("../assets/wR.png");
  textures["black_rook"].loadFromFile("../assets/bR.png");

  textures["white_knight"].loadFromFile("../assets/wN.png");
  textures["black_knight"].loadFromFile("../assets/bN.png");

  textures["white_bishop"].loadFromFile("../assets/wB.png");
  textures["black_bishop"].loadFromFile("../assets/bB.png");

  textures["white_queen"].loadFromFile("../assets/wQ.png");
  textures["black_queen"].loadFromFile("../assets/bQ.png");

  textures["white_king"].loadFromFile("../assets/wK.png");
  textures["black_king"].loadFromFile("../assets/bK.png");
}

void Game::drawPieces()
{
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece = board.getPiece(x, y);

      if (piece != nullptr)
      {
        std::string key = (piece->getColor() == Color::WHITE) ? "white_" : "black_";

        switch (piece->getType())
        {
        case PieceType::PAWN:
          key += "pawn";
          break;
        case PieceType::ROOK:
          key += "rook";
          break;
        case PieceType::KNIGHT:
          key += "knight";
          break;
        case PieceType::BISHOP:
          key += "bishop";
          break;
        case PieceType::QUEEN:
          key += "queen";
          break;
        case PieceType::KING:
          key += "king";
          break;
        default:
          break;
        }

        sf::Sprite sprite;
        sprite.setTexture(textures[key]);

        sprite.setPosition(x * tileSize, y * tileSize);

        sprite.setScale(
            (float)tileSize / sprite.getTexture()->getSize().x,
            (float)tileSize / sprite.getTexture()->getSize().y);

        window.draw(sprite);
      }
    }
  }
}

void Game::drawHighlights()
{
  sf::CircleShape circle(tileSize / 4);
  circle.setFillColor(sf::Color(0, 0, 255, 120));

  for (const Move &m : currentMoves)
  {
    circle.setPosition(
        m.toX * tileSize + tileSize / 4,
        m.toY * tileSize + tileSize / 4);

    window.draw(circle);
  }
}