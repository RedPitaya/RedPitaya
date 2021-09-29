#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  // for usleep
#include <utility>  // for std::pair
#include <array>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCORE_MAX_VALUE 5
#define SCORE_RECTANGE_SIZEX 200
#define NUMBER_OF_BUTTONS 4
#define RACKET_SPEED 16  // number of pixels per frame
#define BALL_RADIUS 16

class Rectangle
{
public:
  Rectangle(int fd, uint32_t addr, uint32_t screenWidth, uint32_t screenHeight) 
  {
    m_map   = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
    m_x     = (static_cast<uint32_t *>(m_map));
    m_y     = (static_cast<uint32_t *>(m_map)) + 1;
    m_sizex = (static_cast<uint32_t *>(m_map)) + 2;
    m_sizey = (static_cast<uint32_t *>(m_map)) + 3;
    m_color = (static_cast<uint32_t *>(m_map)) + 4;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
  };

  ~Rectangle()
  {
    munmap(m_map, sysconf(_SC_PAGESIZE));
  }

  uint32_t GetX()
  {
    return *m_x;
  }

  uint32_t GetY()
  {
    return *m_y;
  }

  uint32_t GetSizeX()
  {
    return *m_sizex;
  }

  uint32_t GetSizeY()
  {
    return *m_sizey;
  }

  void Move(int32_t dx, int32_t dy)
  {
    // Edge checking
    if (static_cast<int16_t>(*m_y + dy) < 0)
      *m_y = 0;
    else if (static_cast<int16_t>(*m_y + dy) + *m_sizey > m_screenHeight)
      *m_y = m_screenHeight - *m_sizey;
    else  
      *m_y += dy;

    *m_x += dx;
  }

  void MoveTo(uint32_t x, uint32_t y)
  {
    *m_x = x;
    *m_y = y;
  }

  void Resize(uint32_t newSizeX, uint32_t newSizeY)
  {
    *m_sizex = newSizeX;
    *m_sizey = newSizeY;
  }

  void SetColor(bool r, bool g, bool b)
  {
    // BGR
    *m_color = r | (g << 1) | (b << 2);
  }

private:
  void* m_map;
  uint32_t* m_x;
  uint32_t* m_y;
  uint32_t* m_sizex;
  uint32_t* m_sizey;
  uint32_t* m_color;
  uint32_t m_screenWidth;
  uint32_t m_screenHeight;
};

class Ball
{
public:
  Ball(int fd, uint32_t addr, uint32_t screenWidth, uint32_t screenHeight, int32_t ballSpeedX, int32_t ballSpeedY)
  {
    m_map   = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
    m_x     = (static_cast<uint32_t *>(m_map));
    m_y     = (static_cast<uint32_t *>(m_map)) + 1;
    m_color = (static_cast<uint32_t *>(m_map)) + 2;
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_ballSpeedX = ballSpeedX;
    m_ballSpeedY = ballSpeedY;
  }

  ~Ball()
  {
    munmap(m_map, sysconf(_SC_PAGESIZE));
  }
  
  // racketLeft and racketRight used for collision detection
  std::pair<bool, bool> Process(Rectangle& racketLeft, Rectangle& racketRight)
  {
    // racket collision
    if (*m_x + m_ballSpeedX < racketLeft.GetX() + racketLeft.GetSizeX())
      if (*m_y + 2 * BALL_RADIUS > racketLeft.GetY())
        if (*m_y < racketLeft.GetY() + racketLeft.GetSizeY())
          m_ballSpeedX *= -1;

    if (*m_x + m_ballSpeedX + 2 * BALL_RADIUS > racketRight.GetX())
      if (*m_y + 2 * BALL_RADIUS > racketRight.GetY())
        if (*m_y < racketRight.GetY() + racketRight.GetSizeY())
          m_ballSpeedX *= -1;

    // Screen collision
    if (static_cast<int16_t>(*m_x + m_ballSpeedX) < 0)
    {
      m_ballSpeedX *= -1;
      return std::make_pair(true, false);
    }

    if (*m_x + m_ballSpeedX + 2 * BALL_RADIUS > m_screenWidth)
    {
      m_ballSpeedX *= -1;
      return std::make_pair(false, true);
    }

    if (static_cast<int16_t>(*m_y + m_ballSpeedY) < 0)
      m_ballSpeedY *= -1;

    if (*m_y + m_ballSpeedY + 2 * BALL_RADIUS > m_screenHeight)
      m_ballSpeedY *= -1;

    *m_x += m_ballSpeedX;
    *m_y += m_ballSpeedY;
    return std::make_pair(false, false);
  }

  void Move(uint32_t dx, uint32_t dy)
  {
    *m_x += dx;
    *m_y += dy;
  }

  void MoveTo(uint32_t x, uint32_t y)
  {
    *m_x = x;
    *m_y = y;
  }

  void SetColor(bool r, bool g, bool b)
  {
    // BGR
    *m_color = r | (g << 1) | (b << 2);
  }
  

private:
  void* m_map;
  uint32_t* m_x;
  uint32_t* m_y;
  uint32_t* m_color;
  uint32_t m_screenWidth;
  uint32_t m_screenHeight;
  int32_t m_ballSpeedX;
  int32_t m_ballSpeedY;
};

class Keyboard
{
public:
  enum class Keys 
  {
    ePlayer1Up,
    ePlayer2Down,
    ePlayer1Down,
    ePlayer2Up,

    eNotPressed,
  };
  
  Keyboard(int fd, uint32_t addr)
  {
    m_map = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
    m_keys = (static_cast<uint32_t *>(m_map));
  }

  uint32_t ValueOfKeys()
  {
    return *m_keys;
  }

  std::pair<Keys, bool> Process()
  {
    // Buttons in the released state are at a high level
    static uint8_t prevState = 0xFF;

    // We calculate the state of which buttons have changed
    uint8_t diff = prevState ^ *m_keys;
    prevState = *m_keys;

    for(size_t i = 0; i < NUMBER_OF_BUTTONS; ++i)
    {
      if ((diff >> i) & 1)
      {
        return std::make_pair(static_cast<Keys>(i), ((*m_keys) >> i) & 1);
      }
    }

    return std::make_pair(Keys::eNotPressed, 0);
  }

private:
  void* m_map;
  uint32_t* m_keys;
};

int main(int argc, char **argv)
{
  int fd;
  const char *name = "/dev/mem";
  printf("Ping pong game started\r\n");

  if((fd = open(name, O_RDWR)) < 0)
  {
    perror("open");
    return 1;
  }

  uint32_t scorePlayer1 = 0;
  uint32_t scorePlayer2 = 0;

  Rectangle racketLeft = Rectangle(fd, 0x43C00000, SCREEN_WIDTH, SCREEN_HEIGHT);
  Rectangle racketRight = Rectangle(fd, 0x43C10000, SCREEN_WIDTH, SCREEN_HEIGHT);
  Rectangle scoreLeft = Rectangle(fd, 0x43C30000, SCREEN_WIDTH, SCREEN_HEIGHT);
  Rectangle scoreRight = Rectangle(fd, 0x43C40000, SCREEN_WIDTH, SCREEN_HEIGHT);
  Keyboard keyboard = Keyboard(fd, 0x43C50000);
  Ball ball = Ball(fd, 0x43C20000, SCREEN_WIDTH, SCREEN_HEIGHT, 11, 5);

  // Default position
  racketLeft.Resize(20, 100);
  racketLeft.SetColor(0, 0, 1);
  
  racketRight.Resize(20, 100);
  racketRight.SetColor(0, 0, 1);

  scoreLeft.Resize(0, 20);
  scoreLeft.MoveTo(SCREEN_WIDTH / 2 - SCORE_MAX_VALUE * (SCORE_RECTANGE_SIZEX / SCORE_MAX_VALUE), 10);
  scoreLeft.SetColor(1, 0, 1);
  
  scoreRight.Resize(0, 20);
  scoreRight.MoveTo(SCREEN_WIDTH / 2, 10);
  scoreRight.SetColor(1, 0, 1);

  ball.SetColor(1, 0, 0);

  std::pair<Keyboard::Keys, bool> key;
  uint32_t player1Speed = 0;
  uint32_t player2Speed = 0;
  bool startGame = false;
  while(true)
  {
    key = keyboard.Process();
    if (key.first != Keyboard::Keys::eNotPressed)
    {
      printf("Key pressed %d with state %d\r\n", static_cast<uint32_t>(key.first), key.second);
      startGame = true;

      switch(key.first)
      {
        case Keyboard::Keys::ePlayer1Up:
          player1Speed = key.second ? 0 : -RACKET_SPEED;
          break;

        case Keyboard::Keys::ePlayer1Down:
          player1Speed = key.second ? 0 : RACKET_SPEED;
          break;
        
        case Keyboard::Keys::ePlayer2Up:
          player2Speed = key.second ? 0 : -RACKET_SPEED;
          break;

        case Keyboard::Keys::ePlayer2Down:
          player2Speed = key.second ? 0 : RACKET_SPEED;
          break;
      }
    }
    if (startGame)
    {
      racketRight.Move(0, player1Speed);
      racketLeft.Move(0, player2Speed);
      auto res = ball.Process(racketLeft, racketRight);
      if (res.first || res.second)
      {
        if (res.first)
          scorePlayer1 += 1;
        
        if (res.second)
          scorePlayer2 += 1;

        startGame = false;
        scoreLeft.Resize(scorePlayer1 * (SCORE_RECTANGE_SIZEX / SCORE_MAX_VALUE), 20);
        scoreRight.Resize(scorePlayer2 * (SCORE_RECTANGE_SIZEX / SCORE_MAX_VALUE), 20);

        if ((scorePlayer1 >= SCORE_MAX_VALUE) || (scorePlayer2 >= SCORE_MAX_VALUE))
          break;
      } 
    }
    else
    {
      racketLeft.MoveTo(10, SCREEN_HEIGHT / 2 - racketLeft.GetSizeY() / 2);
      racketRight.MoveTo(SCREEN_WIDTH - 10 - racketRight.GetSizeX(), SCREEN_HEIGHT / 2 - racketRight.GetSizeY() / 2);
      ball.MoveTo(SCREEN_WIDTH / 2 - BALL_RADIUS, SCREEN_HEIGHT / 2 - BALL_RADIUS);
    }

    usleep(16 * 1000);  // 60fps
  }

  return 0;
}
  