#include "../include/Game.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Game::Game()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
  {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    running = false;
    return;
  }

  if (TTF_Init() != 0)
  {
    std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
    running = false;
    return;
  }

  window = SDL_CreateWindow(
      "Chess Engine",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      800,
      800,
      SDL_WINDOW_SHOWN);

  if (!window)
  {
    std::cerr << "Window creation failed: "
              << SDL_GetError() << std::endl;
    running = false;
    return;
  }

  renderer = SDL_CreateRenderer(
      window,
      -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer)
  {
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE);
  }

  if (!renderer)
  {
    std::cerr << "Renderer creation failed: "
              << SDL_GetError() << std::endl;
    running = false;
    return;
  }

  SDL_SetRenderDrawBlendMode(
      renderer,
      SDL_BLENDMODE_BLEND);

  board.initialize();

  loadTextures();

  font = TTF_OpenFont(
      "assets/Roboto.ttf",
      50);

  if (!font)
  {
    std::cerr << "Error loading font: "
              << TTF_GetError() << std::endl;
  }

  srand(static_cast<unsigned>(time(nullptr)));
}

Game::~Game()
{
  for (auto &pair : textures)
  {
    if (pair.second)
      SDL_DestroyTexture(pair.second);
  }

  textures.clear();

  if (font)
    TTF_CloseFont(font);

  if (renderer)
    SDL_DestroyRenderer(renderer);

  if (window)
    SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}

void Game::run()
{
#ifdef __EMSCRIPTEN__

  emscripten_set_main_loop_arg(
      [](void *arg)
      {
        Game *game = static_cast<Game *>(arg);
        game->gameLoop();
      },
      this,
      0,
      true);

#else

  while (running)
  {
    gameLoop();
  }

#endif
}

void Game::gameLoop()
{
  if (!running)
    return;

  handleEvents();

  if (gameOver)
  {
    render();
    return;
  }

  if (aipending && !gameOver)
  {
    makeAIMove();

    if (isCheckmate(Color::WHITE))
    {
      gameOver = true;
      gameOverMessage = "BLACK WINS!";
    }

    currentTurn = Color::WHITE;
    aipending = false;
  }

  render();
}

void Game::handleEvents()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      running = false;
      return;
    }

    if (gameOver)
      continue;

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
      int x = event.button.x / tileSize;
      int y = event.button.y / tileSize;

      if (x < 0 || x >= 8 || y < 0 || y >= 8)
        continue;

      // SELECT PIECE
      if (selectedX == -1)
      {
        Piece *piece = board.getPiece(x, y);

        if (piece &&
            piece->getColor() == currentTurn)
        {
          selectedX = x;
          selectedY = y;

          auto rawMoves =
              piece->generateMoves(
                  board,
                  x,
                  y);

          currentMoves.clear();

          // FILTER LEGAL MOVES
          for (const Move &m : rawMoves)
          {
            Piece *captured =
                board.getPiece(
                    m.toX,
                    m.toY);

            Piece *moving =
                board.getPiece(
                    m.fromX,
                    m.fromY);

            board.setPiece(
                m.toX,
                m.toY,
                moving);

            board.setPiece(
                m.fromX,
                m.fromY,
                nullptr);

            bool safe =
                !board.isKingInCheck(
                    currentTurn);

            board.setPiece(
                m.fromX,
                m.fromY,
                moving);

            board.setPiece(
                m.toX,
                m.toY,
                captured);

            if (safe)
              currentMoves.push_back(m);
          }
        }
      }

      // MOVE PIECE
      else
      {
        bool moved = false;

        for (const Move &m : currentMoves)
        {
          if (m.toX == x &&
              m.toY == y)
          {
            board.movePiece(m);

            moved = true;

            // PLAYER CHECKMATE
            if (isCheckmate(Color::BLACK))
            {
              gameOver = true;

              gameOverMessage =
                  "WHITE WINS!";

              selectedX = -1;
              selectedY = -1;
              currentMoves.clear();

              return;
            }

            currentTurn = Color::BLACK;

            aipending = true;

            aiMoveTime =
                SDL_GetTicks() + 400;

            break;
          }
        }

        selectedX = -1;
        selectedY = -1;
        currentMoves.clear();

        if (!moved)
        {
          Piece *piece =
              board.getPiece(x, y);

          if (piece &&
              piece->getColor() == currentTurn)
          {
            selectedX = x;
            selectedY = y;

            auto rawMoves =
                piece->generateMoves(
                    board,
                    x,
                    y);

            for (const Move &m : rawMoves)
            {
              Piece *captured =
                  board.getPiece(
                      m.toX,
                      m.toY);

              Piece *moving =
                  board.getPiece(
                      m.fromX,
                      m.fromY);

              board.setPiece(
                  m.toX,
                  m.toY,
                  moving);

              board.setPiece(
                  m.fromX,
                  m.fromY,
                  nullptr);

              bool safe =
                  !board.isKingInCheck(
                      currentTurn);

              board.setPiece(
                  m.fromX,
                  m.fromY,
                  moving);

              board.setPiece(
                  m.toX,
                  m.toY,
                  captured);

              if (safe)
                currentMoves.push_back(m);
            }
          }
        }
      }
    }
  }
}

void Game::render()
{
  SDL_SetRenderDrawColor(
      renderer,
      0,
      0,
      0,
      255);

  SDL_RenderClear(renderer);

  drawBoard();

  drawCheckHighlight();

  drawHighlights();

  drawPieces();

  if (gameOver)
  {
    drawText(
        gameOverMessage,
        250,
        380);
  }

  SDL_RenderPresent(renderer);
}

void Game::drawBoard()
{
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      SDL_Rect square{
          x * tileSize,
          y * tileSize,
          tileSize,
          tileSize};

      if ((x + y) % 2 == 0)
      {
        SDL_SetRenderDrawColor(
            renderer,
            240,
            217,
            181,
            255);
      }
      else
      {
        SDL_SetRenderDrawColor(
            renderer,
            181,
            136,
            99,
            255);
      }

      SDL_RenderFillRect(
          renderer,
          &square);
    }
  }
}

void Game::loadTextures()
{
  textures["white_pawn"] =
      loadTexture("assets/wP.png");

  textures["black_pawn"] =
      loadTexture("assets/bP.png");

  textures["white_rook"] =
      loadTexture("assets/wR.png");

  textures["black_rook"] =
      loadTexture("assets/bR.png");

  textures["white_knight"] =
      loadTexture("assets/wN.png");

  textures["black_knight"] =
      loadTexture("assets/bN.png");

  textures["white_bishop"] =
      loadTexture("assets/wB.png");

  textures["black_bishop"] =
      loadTexture("assets/bB.png");

  textures["white_queen"] =
      loadTexture("assets/wQ.png");

  textures["black_queen"] =
      loadTexture("assets/bQ.png");

  textures["white_king"] =
      loadTexture("assets/wK.png");

  textures["black_king"] =
      loadTexture("assets/bK.png");
}

SDL_Texture *Game::loadTexture(
    const std::string &path)
{
  SDL_Surface *surface =
      IMG_Load(path.c_str());

  if (!surface)
  {
    std::cerr
        << "Failed to load image "
        << path
        << ": "
        << IMG_GetError()
        << std::endl;

    return nullptr;
  }

  SDL_Texture *texture =
      SDL_CreateTextureFromSurface(
          renderer,
          surface);

  SDL_FreeSurface(surface);

  if (!texture)
  {
    std::cerr
        << "Failed to create texture: "
        << SDL_GetError()
        << std::endl;
  }

  return texture;
}

void Game::drawPieces()
{
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece =
          board.getPiece(x, y);

      if (!piece)
        continue;

      std::string key =
          (piece->getColor() == Color::WHITE)
              ? "white_"
              : "black_";

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

      SDL_Texture *texture =
          textures[key];

      if (!texture)
        continue;

      SDL_Rect destination{
          x * tileSize,
          y * tileSize,
          tileSize,
          tileSize};

      SDL_RenderCopy(
          renderer,
          texture,
          nullptr,
          &destination);
    }
  }
}

void Game::drawHighlights()
{
  SDL_Color blue{
      0,
      0,
      255,
      120};

  for (const Move &m : currentMoves)
  {
    drawFilledCircle(
        m.toX * tileSize +
            tileSize / 2,
        m.toY * tileSize +
            tileSize / 2,
        tileSize / 4,
        blue);
  }
}

void Game::drawCheckHighlight()
{
  SDL_SetRenderDrawBlendMode(
      renderer,
      SDL_BLENDMODE_BLEND);

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece =
          board.getPiece(x, y);

      if (!piece)
        continue;

      if (piece->getType() != PieceType::KING)
        continue;

      bool inCheck =
          (piece->getColor() == Color::WHITE &&
           board.isKingInCheck(Color::WHITE)) ||

          (piece->getColor() == Color::BLACK &&
           board.isKingInCheck(Color::BLACK));

      if (inCheck)
      {
        SDL_SetRenderDrawColor(
            renderer,
            255,
            0,
            0,
            100);

        SDL_Rect highlight{
            x * tileSize,
            y * tileSize,
            tileSize,
            tileSize};

        SDL_RenderFillRect(
            renderer,
            &highlight);
      }
    }
  }
}

void Game::drawFilledCircle(
    int cx,
    int cy,
    int radius,
    SDL_Color color)
{
  SDL_SetRenderDrawColor(
      renderer,
      color.r,
      color.g,
      color.b,
      color.a);

  for (int dy = -radius;
       dy <= radius;
       dy++)
  {
    int dx =
        static_cast<int>(
            std::sqrt(
                radius * radius -
                dy * dy));

    SDL_RenderDrawLine(
        renderer,
        cx - dx,
        cy + dy,
        cx + dx,
        cy + dy);
  }
}

void Game::drawText(
    const std::string &text,
    int x,
    int y)
{
  if (!font)
    return;

  SDL_Color color{
      255,
      0,
      0,
      255};

  SDL_Surface *surface =
      TTF_RenderText_Blended(
          font,
          text.c_str(),
          color);

  if (!surface)
    return;

  SDL_Texture *texture =
      SDL_CreateTextureFromSurface(
          renderer,
          surface);

  if (!texture)
  {
    SDL_FreeSurface(surface);
    return;
  }

  SDL_Rect destination{
      x,
      y,
      surface->w,
      surface->h};

  SDL_RenderCopy(
      renderer,
      texture,
      nullptr,
      &destination);

  SDL_DestroyTexture(texture);

  SDL_FreeSurface(surface);
}

bool Game::isCheckmate(Color color)
{
  if (!board.isKingInCheck(color))
    return false;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece =
          board.getPiece(x, y);

      if (!piece)
        continue;

      if (piece->getColor() != color)
        continue;

      auto moves =
          piece->generateMoves(
              board,
              x,
              y);

      for (const Move &m : moves)
      {
        Piece *captured =
            board.getPiece(
                m.toX,
                m.toY);

        Piece *moving =
            board.getPiece(
                m.fromX,
                m.fromY);

        board.setPiece(
            m.toX,
            m.toY,
            moving);

        board.setPiece(
            m.fromX,
            m.fromY,
            nullptr);

        bool safe =
            !board.isKingInCheck(color);

        board.setPiece(
            m.fromX,
            m.fromY,
            moving);

        board.setPiece(
            m.toX,
            m.toY,
            captured);

        if (safe)
          return false;
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
      Piece *piece =
          board.getPiece(x, y);

      if (!piece)
        continue;

      int value =
          getPieceValue(
              piece->getType());

      if (piece->getColor() == Color::BLACK)
        score += value;
      else
        score -= value;
    }
  }

  return score;
}

int Game::minimax(
    int depth,
    int alpha,
    int beta,
    bool isMaximizing)
{
  if (depth == 0)
    return evaluateBoard();

  if (isMaximizing)
  {
    int best = -100000;

    for (int y = 0; y < 8; y++)
    {
      for (int x = 0; x < 8; x++)
      {
        Piece *piece =
            board.getPiece(x, y);

        if (!piece ||
            piece->getColor() != Color::BLACK)
          continue;

        auto moves =
            piece->generateMoves(
                board,
                x,
                y);

        for (const Move &m : moves)
        {
          Piece *captured =
              board.getPiece(
                  m.toX,
                  m.toY);

          Piece *moving =
              board.getPiece(
                  m.fromX,
                  m.fromY);

          board.setPiece(
              m.toX,
              m.toY,
              moving);

          board.setPiece(
              m.fromX,
              m.fromY,
              nullptr);

          if (!board.isKingInCheck(
                  Color::BLACK))
          {
            int score =
                minimax(
                    depth - 1,
                    alpha,
                    beta,
                    false);

            best =
                std::max(
                    best,
                    score);

            alpha =
                std::max(
                    alpha,
                    best);
          }

          board.setPiece(
              m.fromX,
              m.fromY,
              moving);

          board.setPiece(
              m.toX,
              m.toY,
              captured);

          if (beta <= alpha)
            break;
        }
      }
    }

    return best;
  }

  int best = 100000;

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece =
          board.getPiece(x, y);

      if (!piece ||
          piece->getColor() != Color::WHITE)
        continue;

      auto moves =
          piece->generateMoves(
              board,
              x,
              y);

      for (const Move &m : moves)
      {
        Piece *captured =
            board.getPiece(
                m.toX,
                m.toY);

        Piece *moving =
            board.getPiece(
                m.fromX,
                m.fromY);

        board.setPiece(
            m.toX,
            m.toY,
            moving);

        board.setPiece(
            m.fromX,
            m.fromY,
            nullptr);

        if (!board.isKingInCheck(
                Color::WHITE))
        {
          int score =
              minimax(
                  depth - 1,
                  alpha,
                  beta,
                  true);

          best =
              std::min(
                  best,
                  score);

          beta =
              std::min(
                  beta,
                  best);
        }

        board.setPiece(
            m.fromX,
            m.fromY,
            moving);

        board.setPiece(
            m.toX,
            m.toY,
            captured);

        if (beta <= alpha)
          break;
      }
    }
  }

  return best;
}

void Game::makeAIMove()
{
  int bestScore = -100000;

  Move bestMove(
      0,
      0,
      0,
      0);

  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 8; x++)
    {
      Piece *piece =
          board.getPiece(x, y);

      if (!piece ||
          piece->getColor() != Color::BLACK)
        continue;

      auto moves =
          piece->generateMoves(
              board,
              x,
              y);

      for (const Move &m : moves)
      {
        Piece *captured =
            board.getPiece(
                m.toX,
                m.toY);

        Piece *moving =
            board.getPiece(
                m.fromX,
                m.fromY);

        board.setPiece(
            m.toX,
            m.toY,
            moving);

        board.setPiece(
            m.fromX,
            m.fromY,
            nullptr);

        if (!board.isKingInCheck(
                Color::BLACK))
        {
          int score =
              minimax(
                  2,
                  -100000,
                  100000,
                  false);

          if (score > bestScore)
          {
            bestScore = score;
            bestMove = m;
          }
        }

        board.setPiece(
            m.fromX,
            m.fromY,
            moving);

        board.setPiece(
            m.toX,
            m.toY,
            captured);
      }
    }
  }

  board.movePiece(bestMove);
}