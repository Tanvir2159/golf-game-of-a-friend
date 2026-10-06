#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

#define WIDTH 1000
#define HEIGHT 600
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 2000.0f
#define DECELERATION 100.0f
#define STONEWALL 45.0f
#define MOVING_WIDTH 50.0f
#define MOVING_HEIGHT 200.0f
#define MOVING_OBS_SPEED 80.0f

static Vector2 ballPosition = {(WIDTH * 1.5f) / 10.0f, HEIGHT / 2.0f};
static Vector2 ballSpeed = {0.0f, 0.0f};
static Vector2 mouse = {0.0f, 0.0f};
static Vector2 hole = {(WIDTH * 8.5f) / 10.0f, HEIGHT / 2.0f};

static Rectangle movingObstacles[3] = {
    {300.0f, 100.0f, MOVING_WIDTH, MOVING_HEIGHT},
    {500.0f, 300.0f, MOVING_WIDTH, MOVING_HEIGHT},
    {700.0f, 100.0f, MOVING_WIDTH, MOVING_HEIGHT}
};

static float obstacleDirections[3] = {1.0f, -1.0f, 1.0f};
static float obstacleHitCooldown = 0.0f;
static float textDisplayTimer = 0.0f;
static bool hasWon = false;

static void ResolveMovingObstacle(Rectangle obstacle)
{
    if (!CheckCollisionCircleRec(ballPosition, BALL_RADIUS, obstacle))
        return;

    if (obstacleHitCooldown <= 0.0f && Vector2Length(ballSpeed) > STOP_THRESHOLD)
    {
        obstacleHitCooldown = 0.15f;
    }

    float left = obstacle.x;
    float right = obstacle.x + obstacle.width;
    float top = obstacle.y;
    float bottom = obstacle.y + obstacle.height;

    Vector2 contact = {
        Clamp(ballPosition.x, left, right),
        Clamp(ballPosition.y, top, bottom)
    };

    Vector2 normal = Vector2Subtract(ballPosition, contact);

    if (Vector2Length(normal) < 0.0001f)
    {
        float distanceToLeft = ballPosition.x - left;
        float distanceToRight = right - ballPosition.x;
        float distanceToTop = ballPosition.y - top;
        float distanceToBottom = bottom - ballPosition.y;
        float nearest = fminf(fminf(distanceToLeft, distanceToRight),
                              fminf(distanceToTop, distanceToBottom));

        if (nearest == distanceToLeft)       { contact.x = left; normal = (Vector2){-1.0f, 0.0f}; }
        else if (nearest == distanceToRight) { contact.x = right; normal = (Vector2){1.0f, 0.0f}; }
        else if (nearest == distanceToTop)   { contact.y = top; normal = (Vector2){0.0f, -1.0f}; }
        else                                { contact.y = bottom; normal = (Vector2){0.0f, 1.0f}; }
    }

    if (Vector2Length(normal) < 0.0001f)
    {
        normal = (Vector2){0.0f, -1.0f};
    }
    else
    {
        normal = Vector2Normalize(normal);
    }

    ballSpeed = Vector2Reflect(ballSpeed, normal);
    ballPosition = Vector2Add(contact, Vector2Scale(normal, BALL_RADIUS + 0.01f));
}

static void MoveBall(float dt)
{
    int steps = (int)(Vector2Length(ballSpeed) * dt / (BALL_RADIUS * 0.5f)) + 1;
    float stepTime = dt / (float)steps;

    for (int step = 0; step < steps; step++)
    {
        ballPosition = Vector2Add(ballPosition, Vector2Scale(ballSpeed, stepTime));
        for (int i = 0; i < 3; i++)
        {
            ResolveMovingObstacle(movingObstacles[i]);
        }
    }
}

static void ApplyFriction(float dt)
{
    float speedLength = Vector2Length(ballSpeed);
    if (speedLength <= 0.0f)
        return;

    float reducedSpeed = fmaxf(0.0f, speedLength - DECELERATION * dt);
    ballSpeed = Vector2Scale(ballSpeed, reducedSpeed / speedLength);

    if (reducedSpeed < STOP_THRESHOLD)
        ballSpeed = (Vector2){0.0f, 0.0f};
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Texture2D backgroundl5 = LoadTexture("Assets/Textures/level 5 background.png");
    Texture2D movingObs = LoadTexture("Assets/Textures/Moving obstacles.png");
    Font font = LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        obstacleHitCooldown = fmaxf(0.0f, obstacleHitCooldown - dt);

        if (IsKeyPressed(KEY_R))
        {
            ballPosition = (Vector2){(WIDTH * 1.5f) / 10.0f, HEIGHT / 2.0f};
            ballSpeed = (Vector2){0.0f, 0.0f};
            hasWon = false;
            textDisplayTimer = 0.0f;
            movingObstacles[0] = (Rectangle){300.0f, 100.0f, MOVING_WIDTH, MOVING_HEIGHT};
            movingObstacles[1] = (Rectangle){500.0f, 300.0f, MOVING_WIDTH, MOVING_HEIGHT};
            movingObstacles[2] = (Rectangle){700.0f, 100.0f, MOVING_WIDTH, MOVING_HEIGHT};
            obstacleDirections[0] = 1.0f;
            obstacleDirections[1] = -1.0f;
            obstacleDirections[2] = 1.0f;
        }

        for (int obstacleIndex = 0; obstacleIndex < 3; obstacleIndex++)
        {
            Rectangle *obstacle = &movingObstacles[obstacleIndex];
            obstacle->y += obstacleDirections[obstacleIndex] * MOVING_OBS_SPEED * dt;

            if (obstacle->y <= 45.0f)
            {
                obstacle->y = 45.0f;
                obstacleDirections[obstacleIndex] = 1.0f;
            }
            else if (obstacle->y + obstacle->height >= HEIGHT - 45.0f)
            {
                obstacle->y = HEIGHT - 45.0f - obstacle->height;
                obstacleDirections[obstacleIndex] = -1.0f;
            }
        }

        MoveBall(dt);

        if (ballPosition.y - BALL_RADIUS < STONEWALL)
        {
            ballPosition.y = BALL_RADIUS + STONEWALL;
            ballSpeed.y = -ballSpeed.y;
        }
        else if (ballPosition.y + BALL_RADIUS > HEIGHT - STONEWALL)
        {
            ballPosition.y = HEIGHT - BALL_RADIUS - STONEWALL;
            ballSpeed.y = -ballSpeed.y;
        }

        if (ballPosition.x - BALL_RADIUS < STONEWALL)
        {
            ballPosition.x = BALL_RADIUS + STONEWALL;
            ballSpeed.x = -ballSpeed.x;
        }
        else if (ballPosition.x + BALL_RADIUS > WIDTH - STONEWALL)
        {
            ballPosition.x = WIDTH - BALL_RADIUS - STONEWALL;
            ballSpeed.x = -ballSpeed.x;
        }

        for (int i = 0; i < 3; i++)
        {
            ResolveMovingObstacle(movingObstacles[i]);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && Vector2Length(ballSpeed) < STOP_THRESHOLD)
        {
            mouse = GetMousePosition();
            Vector2 shotDirection = Vector2Subtract(ballPosition, mouse);
            float dragDistance = Vector2Length(shotDirection);
            if (dragDistance > 0.0f)
            {
                if (dragDistance > MAX_SPEED)
                    dragDistance = MAX_SPEED;

                shotDirection = Vector2Normalize(shotDirection);
                ballSpeed = Vector2Scale(shotDirection, 9.0f * dragDistance);
            }
        }

        ApplyFriction(dt);

        Vector2 diffHoleBall = Vector2Subtract(ballPosition, hole);
        if (!hasWon && Vector2Length(diffHoleBall) < HOLE_RADIUS)
        {
            hasWon = true;
            textDisplayTimer = 2.0f;
        }

        if (textDisplayTimer > 0.0f)
        {
            textDisplayTimer -= dt;
        }
        else if (hasWon)
        {
            hasWon = false;
        }

        BeginDrawing();
        ClearBackground(DARKGREEN);

        DrawTexturePro(backgroundl5,
                       (Rectangle){0.0f, 0.0f, (float)backgroundl5.width, (float)backgroundl5.height},
                       (Rectangle){0.0f, 0.0f, (float)WIDTH, (float)HEIGHT},
                       (Vector2){0.0f, 0.0f}, 0.0f, WHITE);

        for (int i = 0; i < 3; i++)
        {
            DrawTexturePro(movingObs,
                           (Rectangle){0.0f, 0.0f, (float)movingObs.width, (float)movingObs.height},
                           (Rectangle){movingObstacles[i].x, movingObstacles[i].y, movingObstacles[i].width, movingObstacles[i].height},
                           (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            mouse = GetMousePosition();
            DrawLineV(ballPosition, mouse, BLACK);
        }

        DrawCircleV(hole, HOLE_RADIUS, BLACK);

        if (!hasWon)
        {
            DrawCircleV(ballPosition, BALL_RADIUS + 2.0f, BLACK);
            DrawCircleV(ballPosition, BALL_RADIUS, WHITE);
        }

        if (textDisplayTimer > 0.0f)
        {
            DrawTextEx(font, "congratulations!\n       You Won!\n", (Vector2){100, 100}, 40, 2, WHITE);
        }

        EndDrawing();
    }

    UnloadTexture(backgroundl5);
    UnloadTexture(movingObs);
    UnloadFont(font);
    CloseWindow();
    return 0;
}

