#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 600
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 1000.0f
#define DECELARATION 100.0f
#define OBSTACLE_WIDTH 31.0f
#define OBSTACLE_HEIGHT 353.0f
#define GRAYZONE_DECELERATION 300.0f
#define POND_RADIUS 100.0f

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_HIGHSCORE,
} GameState;

typedef enum {
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
} LevelID;

int highScore = 0;
int score = 0;
int levelScore = 0;
int shotcount = 0;
int startedplaying = 0;
static bool hasWon = false;
float textDisplayTimer = 0.0f;
LevelID currentLevel = LEVEL_1;

Vector2 position = {(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f};
Vector2 speed = {0.0f, 0.0f};
Vector2 mouse = {0.0f, 0.0f};
Vector2 hole = {(WIDTH * 9.0f) / 10.0f, HEIGHT / 2.0f};

Rectangle obstacle_1 = {247.5f, 0.0f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle obstacle_2 = {477.5f, 247.5f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle obstacle_3 = {707.5f, 0.0f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle level3Zone1 = {200.0f, 0.0f, 100.0f, HEIGHT};
Rectangle level3Zone2 = {700.0f, 0.0f, 100.0f, HEIGHT};
Color level3ZoneColor = {90, 90, 90, 160};
Vector2 level4StartPosition = {(WIDTH * 19.0f) / 20.0f, (HEIGHT * 11.0f) / 12.0f};
Vector2 level4SmallPond = {700.0f, 400.0f};
Rectangle level4LargePond = {100.0f, 100.0f, 200.0f, 400.0f};
bool hitPond = false;

bool ButtonPressed(Rectangle bounds, const char* text, float fontSize)
{
    Vector2 mp = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mp, bounds);
    DrawRectangleRec(bounds, hovered ? LIGHTGRAY : GRAY);
    DrawText(text, (int)bounds.x + 20, (int)bounds.y + 10, (int)fontSize, BLACK);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void LoadHighScore(void)
{
    char *text = LoadFileText("highscore.txt");
    if (text != NULL)
    {
        highScore = atoi(text);
        UnloadFileText(text);
    }
}

void SaveHighScore(void)
{
    SaveFileText("highscore.txt", TextFormat("%d", highScore));
}

static void ResolveCollision(Rectangle obstacle)
{
    if (!CheckCollisionCircleRec(position, BALL_RADIUS, obstacle)) return;

    float left = obstacle.x;
    float right = obstacle.x + obstacle.width;
    float top = obstacle.y;
    float bottom = obstacle.y + obstacle.height;

    Vector2 contact = { Clamp(position.x, left, right), Clamp(position.y, top, bottom) };
    Vector2 normal = Vector2Subtract(position, contact);

    if (Vector2Length(normal) < 0.0001f)
    {
        float dLeft = position.x - left;
        float dRight = right - position.x;
        float dTop = position.y - top;
        float dBottom = bottom - position.y;
        float least = fminf(fminf(dLeft, dRight), fminf(dTop, dBottom));

        if (least == dLeft)       { contact.x = left;   normal = (Vector2){ -1.0f, 0.0f }; }
        else if (least == dRight) { contact.x = right;  normal = (Vector2){  1.0f, 0.0f }; }
        else if (least == dTop)   { contact.y = top;    normal = (Vector2){  0.0f, -1.0f }; }
        else                     { contact.y = bottom; normal = (Vector2){  0.0f,  1.0f }; }
    }

    normal = Vector2Normalize(normal);
    speed = Vector2Reflect(speed, normal);
    position = Vector2Add(contact, Vector2Scale(normal, BALL_RADIUS + 0.01f));
}

static void ResolveObstacles(void)
{
    ResolveCollision(obstacle_1);
    ResolveCollision(obstacle_2);
    ResolveCollision(obstacle_3);
}

static void MoveBall(float dt)
{
    int hops = (int)(Vector2Length(speed) * dt / (BALL_RADIUS * 0.5f)) + 1;
    float h = dt / hops;

    for (int i = 0; i < hops; i++)
    {
        position.x += speed.x * h;
        position.y += speed.y * h;
        ResolveObstacles();
    }
}

static void MoveLevel3Ball(float dt)
{
    position.x += speed.x * dt;
    position.y += speed.y * dt;

    if (position.x + BALL_RADIUS > level3Zone1.x && position.x - BALL_RADIUS < level3Zone1.x + level3Zone1.width)
    {
        if (speed.x > 0.0f) speed.x -= GRAYZONE_DECELERATION * dt;
        if (speed.x < 0.0f) speed.x += GRAYZONE_DECELERATION * dt;
    }

    if (position.x + BALL_RADIUS > level3Zone2.x && position.x - BALL_RADIUS < level3Zone2.x + level3Zone2.width)
    {
        if (speed.x > 0.0f) speed.x += GRAYZONE_DECELERATION * dt;
        if (speed.x < 0.0f) speed.x -= GRAYZONE_DECELERATION * dt;
    }
}

static void DrawLevel3Course(void)
{
    DrawRectangleRec(level3Zone1, level3ZoneColor);
    DrawRectangleRec(level3Zone2, level3ZoneColor);
    DrawText("SLOW", (int)level3Zone1.x + 22, 20, 20, WHITE);
    DrawText("FAST", (int)level3Zone2.x + 25, 20, 20, WHITE);
}

static void MoveLevel4Ball(float dt)
{
    position.x += speed.x * dt;
    position.y += speed.y * dt;
}

static bool Level4BallHitPond(void)
{
    return Vector2Distance(position, level4SmallPond) < POND_RADIUS + BALL_RADIUS ||
           CheckCollisionCircleRec(position, BALL_RADIUS, level4LargePond);
}

static void DrawLevel4Course(void)
{
    DrawRectangleRec(level4LargePond, DARKBLUE);
    DrawCircleV(level4SmallPond, POND_RADIUS, DARKBLUE);
    DrawText("POND", (int)level4LargePond.x + 72, 280, 24, WHITE);
}

void SetupLevel(LevelID level)
{
    currentLevel = level;
    position = (Vector2){(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f};
    speed = (Vector2){0.0f, 0.0f};
    startedplaying = 0;
    hasWon = false;
    hitPond = false;
    shotcount = 0;
    levelScore = 0;

    if (level == LEVEL_1)
    {
        hole = (Vector2){(WIDTH * 9.0f) / 10.0f, HEIGHT / 2.0f};
    }
    else if (level == LEVEL_2)
    {
        hole = (Vector2){(WIDTH * 9.0f) / 10.0f, HEIGHT / 2.0f};
    }
    else if (level == LEVEL_3)
    {
        hole = (Vector2){WIDTH / 2.0f, HEIGHT / 2.0f};
    }
    else
    {
        position = level4StartPosition;
        hole = (Vector2){WIDTH / 2.0f, HEIGHT / 2.0f};
    }
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Font font = LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");
    Texture2D grassTexture = LoadTexture("Assets/Textures/grass03.png");
    Texture2D menuTexture = LoadTexture("Assets/Textures/startmenubackground.png");
    LoadHighScore();
    GameState state = STATE_MENU;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        switch (state)
        {
            case STATE_MENU:
                break;

            case STATE_PLAYING:
            {
                if (currentLevel == LEVEL_2)
                {
                    MoveBall(dt);
                }
                else if (currentLevel == LEVEL_3)
                {
                    MoveLevel3Ball(dt);
                }
                else if (currentLevel == LEVEL_4)
                {
                    MoveLevel4Ball(dt);
                }
                else
                {
                    position.x += speed.x * dt;
                    position.y += speed.y * dt;
                }

                if (position.y - BALL_RADIUS < 0)
                {
                    position.y = BALL_RADIUS;
                    speed.y = -speed.y;
                }
                else if (position.y + BALL_RADIUS > HEIGHT)
                {
                    position.y = HEIGHT - BALL_RADIUS;
                    speed.y = -speed.y;
                }

                if (position.x - BALL_RADIUS < 0)
                {
                    position.x = BALL_RADIUS;
                    speed.x = -speed.x;
                }
                else if (position.x + BALL_RADIUS > WIDTH)
                {
                    position.x = WIDTH - BALL_RADIUS;
                    speed.x = -speed.x;
                }

                if (currentLevel == LEVEL_2)
                {
                    ResolveObstacles();
                }

                if (currentLevel == LEVEL_4 && !hasWon && !hitPond && Level4BallHitPond())
                {
                    hitPond = true;
                    speed = (Vector2){0.0f, 0.0f};
                    position = level4StartPosition;
                    textDisplayTimer = 2.0f;
                }

                if (IsKeyPressed(KEY_ESCAPE))
                {
                    int displayedScore = score + levelScore;
                    if (displayedScore > highScore)
                    {
                        highScore = displayedScore;
                        SaveHighScore();
                    }
                    state = STATE_GAMEOVER;
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && Vector2Length(speed) < STOP_THRESHOLD && startedplaying == 0 && !hitPond)
                {
                    startedplaying = 1;
                    shotcount++;
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && startedplaying == 1)
                {
                    mouse = GetMousePosition();
                    DrawLineV(position, mouse, BLACK);
                }

                if (shotcount == 1 && startedplaying == 1) levelScore = 25;
                else if (shotcount > 1 && shotcount <= 5 && startedplaying == 1) levelScore = 20;
                else if (shotcount > 5 && shotcount <= 10 && startedplaying == 1) levelScore = 10;
                else if (shotcount > 10 && startedplaying == 1) levelScore = 5;

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && startedplaying == 1)
                {
                    mouse = GetMousePosition();
                    Vector2 shotdir = { position.x - mouse.x, position.y - mouse.y };
                    float dragDistance = Vector2Length(shotdir);
                    if (dragDistance > MAX_SPEED) dragDistance = MAX_SPEED;
                    if (dragDistance > 0.0f)
                    {
                        shotdir = Vector2Normalize(shotdir);
                        speed.x = 3.0f * shotdir.x * dragDistance;
                        speed.y = 3.0f * shotdir.y * dragDistance;
                    }
                    startedplaying = 0;
                }

                if (speed.x > 0) speed.x -= DECELARATION * (fabsf(speed.x) / Vector2Length(speed)) * dt;
                if (speed.x < 0) speed.x += DECELARATION * (fabsf(speed.x) / Vector2Length(speed)) * dt;
                if (speed.y > 0) speed.y -= DECELARATION * (fabsf(speed.y) / Vector2Length(speed)) * dt;
                if (speed.y < 0) speed.y += DECELARATION * (fabsf(speed.y) / Vector2Length(speed)) * dt;

                if (Vector2Length(speed) < STOP_THRESHOLD)
                {
                    speed.x = 0.0f;
                    speed.y = 0.0f;
                }

                Vector2 diffHoleBall = { position.x - hole.x, position.y - hole.y };
                if (!hasWon && !hitPond && Vector2Length(diffHoleBall) < 10.0f)
                {
                    hasWon = true;
                    speed = (Vector2){0.0f, 0.0f};
                    if (currentLevel == LEVEL_4)
                    {
                        position = level4StartPosition;
                    }
                    else
                    {
                        position = (Vector2){(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f};
                    }
                    textDisplayTimer = 2.0f;
                }

                if (textDisplayTimer > 0.0f)
                {
                    textDisplayTimer -= dt;
                }

                if (textDisplayTimer <= 0.0f && hasWon)
                {
                    score += levelScore;
                    levelScore = 0;

                    if (score > highScore)
                    {
                        highScore = score;
                        SaveHighScore();
                    }

                    if (currentLevel == LEVEL_1)
                    {
                        SetupLevel(LEVEL_2);
                        state = STATE_PLAYING;
                    }
                    else if (currentLevel == LEVEL_2)
                    {
                        SetupLevel(LEVEL_3);
                        state = STATE_PLAYING;
                    }
                    else if (currentLevel == LEVEL_3)
                    {
                        SetupLevel(LEVEL_4);
                        state = STATE_PLAYING;
                    }
                    else
                    {
                        state = STATE_GAMEOVER;
                    }

                    hasWon = false;
                }

                if (textDisplayTimer <= 0.0f && hitPond)
                {
                    hitPond = false;
                }

                break;
            }

            case STATE_GAMEOVER:
                if (IsKeyPressed(KEY_ENTER)) state = STATE_MENU;
                break;

            case STATE_HIGHSCORE:
                if (IsKeyPressed(KEY_BACKSPACE)) state = STATE_MENU;
                break;
        }

        BeginDrawing();
        switch (state)
        {
            case STATE_MENU:
            {
                ClearBackground(RAYWHITE);
                DrawTexturePro(
                    menuTexture,
                    (Rectangle){0, 0, (float)menuTexture.width, (float)menuTexture.height},
                    (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE);

                DrawText("GOLF MANIA", 300, 100, 40, DARKBLUE);

                if (ButtonPressed((Rectangle){250, 220, 200, 50}, "Start", 20))
                {
                    score = 0;
                    SetupLevel(LEVEL_1);
                    state = STATE_PLAYING;
                }

                if (ButtonPressed((Rectangle){250, 290, 200, 50}, "High Score", 20))
                {
                    state = STATE_HIGHSCORE;
                }

                if (ButtonPressed((Rectangle){250, 360, 200, 50}, "Quit", 20))
                {
                    CloseWindow();
                    return 0;
                }
                break;
            }

            case STATE_PLAYING:
            {
                ClearBackground(BROWN);
                DrawTexturePro(
                    grassTexture,
                    (Rectangle){0, 0, (float)grassTexture.width, (float)grassTexture.height},
                    (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE);

                DrawText(TextFormat("Score: %d", score + levelScore), 10, 10, 20, BLACK);
                DrawText(TextFormat("Shot Count: %d", shotcount), 10, 40, 20, BLACK);
                DrawText(TextFormat("Level: %d", currentLevel + 1), 10, 70, 20, BLACK);
                DrawText("Press ESC to end", 10, 100, 20, GRAY);

                DrawCircleV(hole, HOLE_RADIUS, BLACK);

                if (currentLevel == LEVEL_2)
                {
                    DrawRectangleRec(obstacle_1, BLACK);
                    DrawRectangle(250.0f, 0.0f, OBSTACLE_WIDTH - 6.0f, OBSTACLE_HEIGHT - 3.0f, GRAY);
                    DrawRectangleRec(obstacle_2, BLACK);
                    DrawRectangle(480.0f, 250.0f, OBSTACLE_WIDTH - 6.0f, OBSTACLE_HEIGHT - 3.0f, GRAY);
                    DrawRectangleRec(obstacle_3, BLACK);
                    DrawRectangle(710.0f, 0.0f, OBSTACLE_WIDTH - 6.0f, OBSTACLE_HEIGHT - 3.0f, GRAY);
                }
                else if (currentLevel == LEVEL_3)
                {
                    DrawLevel3Course();
                }
                else if (currentLevel == LEVEL_4)
                {
                    DrawLevel4Course();
                }

                if (!hasWon)
                {
                    DrawCircle(position.x, position.y, BALL_RADIUS + 2.0f, BLACK);
                    DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
                }

                if (startedplaying == 1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    mouse = GetMousePosition();
                    DrawLineV(position, mouse, BLACK);
                }

                if (textDisplayTimer > 0.0f)
                {
                    const char *message = hitPond ? "Oh no!\nYou hit the pond!" : "congratulations!\n       You Won!\n";
                    DrawTextEx(font, message, (Vector2){100, 100}, 40, 2, WHITE);
                }

                break;
            }

            case STATE_HIGHSCORE:
            {
                ClearBackground(RAYWHITE);
                const char *highScoreText = TextFormat("High Score: %d", highScore);
                const char *backText = "Press BACKSPACE to go back";
                DrawText(highScoreText, (GetScreenWidth() - MeasureText(highScoreText, 30)) / 2, GetScreenHeight() / 2 - 100, 30, MAROON);
                DrawText(backText, (GetScreenWidth() - MeasureText(backText, 20)) / 2, GetScreenHeight() / 2 + 50, 20, GRAY);
                break;
            }

            case STATE_GAMEOVER:
            {
                ClearBackground(RAYWHITE);
                DrawText(TextFormat("Game Over! Score: %d", score + levelScore), GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 - 50, 30, RED);
                shotcount = 0;
                DrawText("Press ENTER for menu", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 + 50, 20, GRAY);
                break;
            }
        }

        EndDrawing();
    }

    UnloadFont(font);
    UnloadTexture(grassTexture);
    UnloadTexture(menuTexture);
    CloseWindow();
    return 0;
}