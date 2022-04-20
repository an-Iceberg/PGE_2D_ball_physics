#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct s_Ball {
  // Position
  float px, py;

  // Velocity
  float vx, vy;

  // Acceleration
  float ax, ay;

  float radius;

  int id;
};

class Example : public olc::PixelGameEngine
{
public:
  Example()
  {
    sAppName = "Circle physics";
  }

private:
  std::vector<s_Ball> vBalls;
  s_Ball* pSelectedBall = nullptr;

  void AddBall(float x, float y, float r = 15.0f)
  {
    s_Ball b;
    b.px = x; b.py = y;
    b.vx = 0; b.vy = 0;
    b.ax = 0; b.ay = 0;
    b.radius = r;

    b.id = vBalls.size();
    vBalls.push_back(b);
  }

public:
  bool OnUserCreate() override
  {
    srand(time(nullptr));

    for (int i = 0; i < 40; i++) {
      AddBall(rand() % ScreenWidth(), rand() % ScreenHeight(), rand()%30+5);
    }
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    UserInput();
    BallCollisionAndPhysics(fElapsedTime);
    DrawingRoutine();
    return true;
  }

  void UserInput()
  {
    if (GetMouse(0).bPressed || GetMouse(1).bPressed)
    {
      pSelectedBall = nullptr;
      for (auto& ball : vBalls)
      {
        if (IsPointInCircle(ball.px, ball.py, ball.radius, GetMouseX(), GetMouseY()))
        {
          pSelectedBall = &ball;

          break;
        }
      }
    }

    if (GetMouse(0).bHeld)
    {
      if (pSelectedBall != nullptr)
      {
        pSelectedBall->px = GetMouseX();
        pSelectedBall->py = GetMouseY();
      }
    }

    if (GetMouse(0).bReleased)
    {
      pSelectedBall = nullptr;
    }

    if (GetMouse(1).bReleased)
    {
      if (pSelectedBall != nullptr)
      {
        pSelectedBall->vx = 5.0f * ((pSelectedBall->px) - float(GetMouseX()));
        pSelectedBall->vy = 5.0f * ((pSelectedBall->py) - float(GetMouseY()));
      }

      pSelectedBall = nullptr;
    }
  }

  void BallCollisionAndPhysics(float fElapsedTime)
  {
    std::vector<std::pair<s_Ball*, s_Ball*>> vCollidingBalls;

    // === Physics ===
    for (auto& ball : vBalls)
    {
      ball.ax = -ball.vx * 0.8f;
      ball.ay = -ball.vy * 0.8f;
      ball.vx += ball.ax * fElapsedTime;
      ball.vy += ball.ay * fElapsedTime;
      ball.px += ball.vx * fElapsedTime;
      ball.py += ball.vy * fElapsedTime;

      // Making sure the balls remain in screen space
      if (ball.px < 0)
      {
        ball.px += float(ScreenWidth());
      }
      if (ball.px >= ScreenWidth())
      {
        ball.px -= float(ScreenWidth());
      }
      if (ball.py < 0)
      {
        ball.py += float(ScreenHeight());
      }
      if (ball.py >= ScreenHeight())
      {
        ball.py -= float(ScreenHeight());
      }

      if (fabs(ball.vx * ball.vx + ball.vy * ball.vy) < 0.01f)
      {
        ball.vx = 0;
        ball.vy = 0;
      }
    }

    // === Collision ===
    for (auto& ball : vBalls)
    {
      for (auto& target : vBalls)
      {
        if (ball.id != target.id)
        {
          if (DoCirclesOverlap(ball.px, ball.py, ball.radius, target.px, target.py, target.radius))
          {
            // This is for the physics part
            vCollidingBalls.push_back({ &ball, &target });

            float fDistance = sqrtf((ball.px - target.px) * (ball.px - target.px) + (ball.py - target.py) * (ball.py - target.py));
            float fOverlap = 0.5f * (fDistance - ball.radius - target.radius);

            ball.px -= fOverlap * (ball.px - target.px) / fDistance;
            ball.py -= fOverlap * (ball.py - target.py) / fDistance;

            target.px += fOverlap * (ball.px - target.px) / fDistance;
            target.py += fOverlap * (ball.py - target.py) / fDistance;
          }
        }
      }
    }

    // === Dynamic collision (physical collision) ===
    for (auto c : vCollidingBalls)
    {
      s_Ball* b1 = c.first;
      s_Ball* b2 = c.second;

      float fDistance = sqrtf((b1->px - b2->px) * (b1->px - b2->px) + (b1->py - b2->py) * (b1->py - b2->py));

      // Normal vector
      float nx = (b2->px - b1->px) / fDistance;
      float ny = (b2->py - b1->py) / fDistance;

      // Tangent vector
      float tx = -ny;
      float ty = nx;

      // Dot product tangent
      float dpTan1 = b1->vx * tx + b1->vy * ty;
      float dpTan2 = b2->vx * tx + b2->vy * ty;

      // Dot product normal
      float dpNorm1 = b1->vx * nx + b1->vy * ny;
      float dpNorm2 = b2->vx * nx + b2->vy * ny;

      // Conservation of momentum in 1D
      float m1 = (dpNorm1 * (b1->radius - b2->radius) + 2.0f * b2->radius * dpNorm2) / (b1->radius + b2->radius);
      float m2 = (dpNorm2 * (b2->radius - b1->radius) + 2.0f * b1->radius * dpNorm1) / (b1->radius + b2->radius);

      b1->vx = tx * dpTan1 + nx * m1;
      b1->vy = ty * dpTan1 + ny * m1;
      b2->vx = tx * dpTan2 + nx * m2;
      b2->vy = ty * dpTan2 + ny * m2;
    }
  }

  void DrawingRoutine()
  {
    Clear(olc::DARK_BLUE);

    for (auto const& ball : vBalls)
    {
      DrawCircle(ball.px, ball.py, ball.radius);
    }

    if(pSelectedBall != nullptr)
    {
      DrawLine(pSelectedBall->px, pSelectedBall->py, GetMouseX(), GetMouseY(), olc::GREEN);
    }
  }

  bool DoCirclesOverlap(float x1, float y1, float r1, float x2, float y2, float r2)
  {
    return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
  }

  bool IsPointInCircle(float x1, float y1, float r1, float px, float py)
  {
    return fabs((x1 - px) * (x1 - px) + (y1 - py) * (y1 - py)) < (r1 * r1);
  }
};

int main()
{
  Example demo;
  if (demo.Construct(1000, 800, 1, 1))
    demo.Start();
  return 0;
}
