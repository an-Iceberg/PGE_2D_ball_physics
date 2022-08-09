#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct ball
{
  olc::vf2d position;
  olc::vf2d velocity;
  olc::vf2d acceleration;
  float radius;
  int id;
};

// TODO: add comments
class Example : public olc::PixelGameEngine
{
public:
  Example()
  {
    sAppName = "Circle physics";
  }

private:
  std::vector<ball> balls;
  ball* selectedBall = nullptr;
  olc::vi2d mouse;

  void AddBall(float x, float y, float radius = 15.0f)
  {
    ball ball;
    ball.position = {x, y};
    ball.velocity = {0, 0};
    ball.acceleration = {0, 0};
    ball.radius = radius;
    ball.id = balls.size();

    balls.push_back(ball);
  }

public:
  bool OnUserCreate() override
  {
    srand(time(nullptr));

    // Creating a bunch of randomly sized and positioned balls
    for (int i = 0; i < 40; i++)
    {
      AddBall(rand() % ScreenWidth(), rand() % ScreenHeight(), rand() % 30 + 5);
    }

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    mouse = {GetMouseX(), GetMouseY()};

    UserInput();
    BallCollisionAndPhysics(fElapsedTime);
    DrawingRoutine();
    return true;
  }

  void UserInput()
  {
    if (GetMouse(0).bPressed or GetMouse(1).bPressed)
    {
      selectedBall = nullptr;

      for (auto& ball : balls)
      {
        if (IsPointInCircle(ball.position.x, ball.position.y, ball.radius, mouse.x, mouse.y))
        {
          selectedBall = &ball;

          break;
        }
      }
    }

    if (GetMouse(0).bHeld and selectedBall != nullptr)
    {
      selectedBall->position = mouse;
    }

    if (GetMouse(0).bReleased)
    {
      selectedBall = nullptr;
    }

    if (GetMouse(1).bReleased and selectedBall != nullptr)
    {
      selectedBall->velocity = 5.0f * (selectedBall->position - mouse);

      selectedBall = nullptr;
    }
  }

  void BallCollisionAndPhysics(float fElapsedTime)
  {
    // Temporarily holds all the balls that are colliding
    std::vector<std::pair<ball*, ball*>> collidingBalls;

    // === Physics ===
    for (auto& ball : balls)
    {
      // Updating the physical properties of each ball
      ball.acceleration = -ball.velocity * 0.8f;
      ball.velocity += ball.acceleration * fElapsedTime;
      ball.position += ball.velocity * fElapsedTime;

      // Making sure the balls remain in screen space
      if (ball.position.x < 0)
      {
        ball.position.x += float(ScreenWidth());
      }
      if (ball.position.x >= ScreenWidth())
      {
        ball.position.x -= float(ScreenWidth());
      }
      if (ball.position.y < 0)
      {
        ball.position.y += float(ScreenHeight());
      }
      if (ball.position.y >= ScreenHeight())
      {
        ball.position.y -= float(ScreenHeight());
      }

      // If the ball reaches a slow enough velocity it gets clamped to 0
      if (fabs(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y) < 0.5f)
      {
        ball.velocity = {0.0f, 0.0f};
      }
    }

    // Calculates collisions when the user moves a ball around
    for (auto& ball : balls)
    {
      for (auto& target : balls)
      {
        if (ball.id != target.id)
        {
          if (DoCirclesOverlap(ball.position.x, ball.position.y, ball.radius, target.position.x, target.position.y, target.radius))
          {
            // This is for the physics part
            collidingBalls.push_back({&ball, &target});

            float distance = sqrtf((ball.position.x - target.position.x) * (ball.position.x - target.position.x) + (ball.position.y - target.position.y) * (ball.position.y - target.position.y));
            float overlap = 0.5f * (distance - ball.radius - target.radius);

            ball.position -= overlap * (ball.position - target.position) / distance;

            target.position += overlap * (ball.position - target.position) / distance;
          }
        }
      }
    }

    // Calculates collisions that happen when two moving balls collide
    for (auto& collidingBall : collidingBalls)
    {
      ball* ball1 = collidingBall.first;
      ball* ball2 = collidingBall.second;

      float fDistance = sqrtf((ball1->position.x - ball2->position.x) * (ball1->position.x - ball2->position.x) + (ball1->position.y - ball2->position.y) * (ball1->position.y - ball2->position.y));

      olc::vf2d normalVector = (ball2->position - ball1->position) / fDistance;

      olc::vf2d tangentVector = {-normalVector.y, normalVector.x};

      float dotProductTangent1 = ball1->velocity.x * tangentVector.x + ball1->velocity.y * tangentVector.y;
      float dotProductTangent2 = ball2->velocity.x * tangentVector.x + ball2->velocity.y * tangentVector.y;

      float dotProductNormal1 = ball1->velocity.x * normalVector.x + ball1->velocity.y * normalVector.y;
      float dotProductNormal2 = ball2->velocity.x * normalVector.x + ball2->velocity.y * normalVector.y;

      // Conservation of momentum in 1D
      float m1 = (dotProductNormal1 * (ball1->radius - ball2->radius) + 2.0f * ball2->radius * dotProductNormal2) / (ball1->radius + ball2->radius);
      float m2 = (dotProductNormal2 * (ball2->radius - ball1->radius) + 2.0f * ball1->radius * dotProductNormal1) / (ball1->radius + ball2->radius);

      ball1->velocity = tangentVector * dotProductTangent1 + normalVector * m1;
      ball2->velocity = tangentVector * dotProductTangent2 + normalVector * m2;
    }
  }

  void DrawingRoutine()
  {
    Clear(olc::VERY_DARK_CYAN);

    for (auto const& ball : balls)
    {
      DrawCircle(ball.position, ball.radius);
    }

    if(selectedBall != nullptr)
    {
      DrawLine(selectedBall->position, mouse, olc::Pixel(255, 128, 0));
    }
  }

  bool DoCirclesOverlap(float x1, float y1, float r1, float x2, float y2, float r2)
  {
    return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
  }

  bool IsPointInCircle(float x1, float y1, float r1, float positionX, float positionY)
  {
    return fabs((x1 - positionX) * (x1 - positionX) + (y1 - positionY) * (y1 - positionY)) < (r1 * r1);
  }
};

int main()
{
  Example demo;

  if (demo.Construct(1000, 800, 1, 1))
  {
    demo.Start();
  }

  return 0;
}
