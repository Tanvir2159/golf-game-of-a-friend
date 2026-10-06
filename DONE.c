#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 1000
#define HEIGHT 600
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 1000.0f
#define DECELARATION 100.0f
#define OBSTACLE_WIDTH 31.0f
#define OBSTACLE_HEIGHT 353.0f
#define GRAYZONE_DECELERATION 800.0f
#define POND_RADIUS 100.0f
#define STONEWALL 45.0f
#define MOVING_WIDTH 50.0f
#define MOVING_HEIGHT 200.0f
#define MOVING_OBS_SPEED 80.0f
#define PIPE_EXIT_SPEED 100.0f   // max speed when coming out of the pipe
#define PIPE_TRAVEL_SPEED 1100.0f // how fast the ball runs through the pipe
#define PIPE_PATH_POINTS 12
#define PORTAL_RADIUS 25.0f
#define LEVEL_COUNT 7
#define HUD_HEIGHT 34

typedef enum {
    STATE_MENU,
    STATE_LEVEL_SELECT,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_HIGHSCORE,
    STATE_LEADERBOARD,
    STATE_ACKNOWLEDGEMENT,
    STATE_LEVEL_FAILED,
} GameState;

typedef enum {
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_5,
    LEVEL_6,
    LEVEL_7,
} LevelID;

typedef enum {
    GAME_MODE_SINGLE,
    GAME_MODE_TWO_PLAYER,
} GameMode;

#define MAX_PLAYER_NAME_LENGTH 20
#define MAX_LEADERBOARD_ENTRIES 10

typedef struct {
    char name[MAX_PLAYER_NAME_LENGTH];
    int points;
} LeaderboardEntry;

// colours used by every screen so the menus all look the same
Color popYellow = {255, 214, 51, 255};
Color popGreen = {76, 187, 23, 255};
Color darkGreen = {18, 64, 28, 255};
Color popRed = {230, 57, 70, 255};
Color darkRed = {80, 8, 16, 255};
Color popOrange = {255, 140, 26, 255};
Color darkOrange = {90, 40, 0, 255};
Color panelColor = {12, 40, 20, 225};

int highScore = 0;
int score = 0;
int levelScore = 0;
int shotcount = 0;
int startedplaying = 0;
int activePlayer = 0;
int playerScores[2] = {0, 0};
int playerShotCount[2] = {0, 0};
int noShotsPlayer = -1;
bool showNoShotsMessage = false;
float noShotsMessageTimer = 0.0f;
bool playerExhausted[2] = {false, false};
char playerNames[2][MAX_PLAYER_NAME_LENGTH] = {"", ""};
int activePlayerNameField = 0;
bool showTwoPlayerNameEntry = false;
bool playerFinishedCurrentLevel[2] = {false, false};
LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES];
int leaderboardCount = 0;
bool leaderboardRecorded = false;
float nameEntryBlinkTimer = 0.0f;
Vector2 playerFinalBallPosition[2] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
static bool hasWon = false;
float textDisplayTimer = 0.0f;
GameMode gameMode = GAME_MODE_SINGLE;
LevelID currentLevel = LEVEL_1;
GameState state = STATE_MENU;
bool quitRequested = false;
bool anyButtonHovered = false;

Vector2 position = {(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f};
Vector2 speed = {0.0f, 0.0f};
Vector2 mouse = {0.0f, 0.0f};
Vector2 hole = {(WIDTH * 9.0f) / 10.0f, HEIGHT / 2.0f};

// where the ball starts and where the hole is, one entry per level
Vector2 levelStart[LEVEL_COUNT] = {
    {(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f},            // level 1
    {(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f},            // level 2
    {(WIDTH * 1.0f) / 10.0f, HEIGHT / 2.0f},            // level 3
    {(WIDTH * 19.0f) / 20.0f, (HEIGHT * 11.0f) / 12.0f}, // level 4
    {(WIDTH * 1.5f) / 10.0f, HEIGHT / 2.0f},            // level 5
    {WIDTH / 10.0f, HEIGHT / 2.0f},                     // level 6
    {WIDTH / 10.0f, HEIGHT / 4.0f},                     // level 7
};
Vector2 levelHole[LEVEL_COUNT] = {
    {(WIDTH * 9.0f) / 10.0f, HEIGHT / 2.0f},
    {(WIDTH * 9.0f) / 10.0f, HEIGHT / 2.0f},
    {WIDTH / 2.0f, HEIGHT / 2.0f},
    {WIDTH / 2.0f, HEIGHT / 2.0f},
    {(WIDTH * 8.5f) / 10.0f, HEIGHT / 2.0f},
    {(WIDTH * 9.0f) / 10.0f, HEIGHT / 3.0f},
    {(WIDTH * 9.0f) / 10.0f, (HEIGHT * 4.5f) / 6.0f},
};

// level 2
Rectangle obstacle_1 = {247.5f, 0.0f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle obstacle_2 = {477.5f, 247.5f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle obstacle_3 = {707.5f, 0.0f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};

// level 3
Rectangle level3Zone1 = {200.0f, 0.0f, 100.0f, HEIGHT};
Rectangle level3Zone2 = {700.0f, 0.0f, 100.0f, HEIGHT};
Color level3ZoneColor = {90, 90, 90, 190};

// level 4
Vector2 level4SmallPond = {700.0f, 400.0f};
Rectangle level4LargePond = {100.0f, 100.0f, 200.0f, 400.0f};

// level 5 (planks moving up and down)
Rectangle level5Obstacles[3];
float level5Directions[3];

// level 6 (spikes and a pipe)
Rectangle level6LeftSpikes[4] = {
    {200.0f, 0.0f,   100.0f, 130.0f},
    {200.0f, 130.0f, 100.0f, 130.0f},
    {200.0f, 325.0f, 100.0f, 140.0f},
    {200.0f, 465.0f, 100.0f, 140.0f}
};
Rectangle level6RightSpikes[4] = {
    {650.0f, 0.0f,   100.0f, 140.0f},
    {650.0f, 140.0f, 100.0f, 145.0f},
    {650.0f, 345.0f, 100.0f, 130.0f},
    {650.0f, 470.0f, 100.0f, 130.0f}
};
Rectangle level6LeftMouth = {200.0f, 260.0f, 100.0f, 65.0f};
Rectangle level6RightMouth = {650.0f, 285.0f, 100.0f, 60.0f};
bool canTeleport = true;
bool inPipe = false;
bool pipeForward = true;   // true = went in on the left, comes out on the right
float pipeDistance = 0.0f; // how far along the pipe the ball is
float pipeEntrySpeed = 0.0f; // speed the ball had when it went in
// middle line of "Pipe L6.png" from the left opening to the right opening (screen coordinates)
Vector2 pipePath[PIPE_PATH_POINTS] = {
    {222.0f, 293.0f}, {299.6f, 291.6f}, {299.6f, 193.5f}, {364.5f, 193.5f},
    {364.5f, 352.1f}, {574.6f, 352.1f}, {574.6f, 203.1f}, {479.7f, 203.1f},
    {479.7f, 411.8f}, {640.0f, 411.8f}, {640.0f, 317.2f}, {736.3f, 317.7f}
};
Rectangle pipeDrawRect = {200.0f, 152.75f, 550.0f, 300.0f};

// level 7 (three sections joined by portals)
Rectangle level7Walls[2] = {
    {0.0f, 289.0f, WIDTH, 18.0f},     // top sections | bottom section
    {490.0f, 0.0f, 20.0f, 300.0f}     // section 1 | section 2
};
Rectangle level7Pillars[6] = {
    {240.0f, 18.0f, 25.0f, 150.0f},
    {350.0f, 140.0f, 25.0f, 150.0f},
    {732.5f, 78.5f, 25.0f, 150.0f},
    {300.0f, 307.0f, 25.0f, 170.0f},
    {550.0f, 412.0f, 25.0f, 170.0f},
    {750.0f, 307.0f, 25.0f, 170.0f}
};
Vector2 portalIn[2] = {{440.0f, 230.0f}, {930.0f, 240.0f}};
Vector2 portalOut[2] = {{560.0f, 156.0f}, {70.0f, 445.0f}};
int portalFrame = 0;
float portalFrameTimer = 0.0f;

// hazards (pond on level 4, spikes on level 6) share one message
bool hitHazard = false;
const char *hazardMessage = "";
const char *gameOverReason = ""; // shown on the game over screen (spikes)

Font uiFont;
Font myFont;
Texture2D arrow1;
Texture2D arrow2;
Texture2D congratulationsTexture;
Texture2D pillarL2Texture;
Texture2D largePond;
Texture2D smallPond;
Texture2D GameOver;
Texture2D buttonTexture;
Texture2D movingObsTexture;
Texture2D spikeLeftTexture;
Texture2D spikeRightTexture;
Texture2D pipeTexture;
Texture2D portalTexture;
Texture2D levelBackgroundTexture;
Texture2D level5Background;
Texture2D level6Background;
Texture2D level7Background;
Texture2D menuTexture;

// only the stone bar part of "Menu Options.png" (the rest is transparent)
Rectangle buttonSource = {0.0f, 212.0f, 2172.0f, 306.0f};

Music startingMenu;
Music levelMusic;
Sound ballPuttSound;
Sound ballHitSound;
Sound levelCompletion;
Sound levelFailed;
Sound waterSplash;
Sound wallHit;
Sound obsHitl2;
Sound spikeHit;
Sound whoosh;

bool levelFailedSoundPlayed = false;
float obstacleHitCooldown = 0.0f;

// ---------- drawing helpers (same look on every screen) ----------

static void DrawFullScreen(Texture2D texture)
{
    DrawTexturePro(
        texture,
        (Rectangle){0, 0, (float)texture.width, (float)texture.height},
        (Rectangle){0, 0, (float)WIDTH, (float)HEIGHT},
        (Vector2){0, 0},
        0.0f,
        WHITE);
}

// big title text: white letters, coloured 3D edge, dark outline (like the Congratulations picture)
// the font only has letters, digits and spaces, so no punctuation here
 void DrawBannerText(const char *text, float y, float fontSize, Color edge, Color outline)
{
    float spacing = fontSize / 12.0f;
    Vector2 size = MeasureTextEx(uiFont, text, fontSize, spacing);
    Vector2 pos = {(WIDTH - size.x) / 2.0f, y};
    float thick = fontSize / 14.0f;

    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 2; dy++)
        {
            DrawTextEx(uiFont, text, (Vector2){pos.x + dx * thick, pos.y + dy * thick}, fontSize, spacing, outline);
        }
    }
    DrawTextEx(uiFont, text, (Vector2){pos.x, pos.y + thick}, fontSize, spacing, edge);
    DrawTextEx(uiFont, text, pos, fontSize, spacing, RAYWHITE);
}

 void DrawCenteredText(const char *text, int y, int fontSize, Color color)
{
    float spacing = 1.0f;
    Vector2 textSize = MeasureTextEx(myFont, text, (float)fontSize, spacing);
    Vector2 pos = {(WIDTH - textSize.x) / 2.0f, (float)y};
    DrawTextEx(myFont, text, pos, (float)fontSize, spacing, color);
}

// stone button made from "Menu Options.png"
bool ButtonPressed(Rectangle bounds, const char* text, float fontSize)
{
    Vector2 mp = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mp, bounds);
    if (hovered) anyButtonHovered = true;

    Rectangle drawBounds = bounds;
    if (hovered) drawBounds.y -= 3.0f;

    DrawRectangleRounded((Rectangle){bounds.x + 4.0f, bounds.y + 6.0f, bounds.width, bounds.height}, 0.35f, 8, Fade(BLACK, 0.45f));
    DrawTexturePro(buttonTexture, buttonSource, drawBounds, (Vector2){0, 0}, 0.0f, hovered ? (Color){255, 238, 190, 255} : WHITE);
    if (hovered) DrawRectangleRoundedLinesEx(drawBounds, 0.35f, 8, 3.0f, popYellow);

    float spacing = 2.0f;
    Vector2 size = MeasureTextEx(uiFont, text, fontSize, spacing);
    while (size.x > bounds.width - 30.0f && fontSize > 10.0f)
    {
        fontSize -= 1.0f;
        size = MeasureTextEx(uiFont, text, fontSize, spacing);
    }
    Vector2 textPos = {drawBounds.x + (drawBounds.width - size.x) / 2.0f, drawBounds.y + (drawBounds.height - size.y) / 2.0f};
    DrawTextEx(uiFont, text, (Vector2){textPos.x + 2.0f, textPos.y + 3.0f}, fontSize, spacing, Fade(BLACK, 0.75f));
    DrawTextEx(uiFont, text, textPos, fontSize, spacing, hovered ? popYellow : RAYWHITE);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static Texture2D LevelBackground(LevelID level)
{
    if (level == LEVEL_5) return level5Background;
    if (level == LEVEL_6) return level6Background;
    if (level == LEVEL_7) return level7Background;
    return levelBackgroundTexture;
}

// ---------- high score and leaderboard ----------

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

void SortLeaderboard(void)
{
    for (int i = 0; i < leaderboardCount - 1; i++)
    {
        for (int j = i + 1; j < leaderboardCount; j++)
        {
            if (leaderboard[j].points > leaderboard[i].points)
            {
                LeaderboardEntry temporary = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temporary;
            }
        }
    }
}

void LoadLeaderboard(void)
{
    leaderboardCount = 0;
    FILE *file = fopen("leaderboard.txt", "r");
    if (file == NULL) return;

    while (leaderboardCount < MAX_LEADERBOARD_ENTRIES &&
           fscanf(file, "%19[^|]|%d\n", leaderboard[leaderboardCount].name, &leaderboard[leaderboardCount].points) == 2)
    {
        leaderboardCount++;
    }

    fclose(file);
    SortLeaderboard();
}

void SaveLeaderboard(void)
{
    FILE *file = fopen("leaderboard.txt", "w");
    if (file == NULL) return;

    for (int i = 0; i < leaderboardCount; i++)
    {
        fprintf(file, "%s|%d\n", leaderboard[i].name, leaderboard[i].points);
    }

    fclose(file);
}

void AddLeaderboardEntry(const char *name, int points)
{
    if (name == NULL || name[0] == '\0') return;

    if (leaderboardCount < MAX_LEADERBOARD_ENTRIES)
    {
        snprintf(leaderboard[leaderboardCount].name, MAX_PLAYER_NAME_LENGTH, "%s", name);
        leaderboard[leaderboardCount].points = points;
        leaderboardCount++;
    }
    else if (points > leaderboard[leaderboardCount - 1].points)
    {
        snprintf(leaderboard[leaderboardCount - 1].name, MAX_PLAYER_NAME_LENGTH, "%s", name);
        leaderboard[leaderboardCount - 1].points = points;
    }

    SortLeaderboard();
    SaveLeaderboard();
}

void RecordCurrentGameOnLeaderboard(void)
{
    if (leaderboardRecorded) return;

    if (gameMode == GAME_MODE_TWO_PLAYER)
    {
        AddLeaderboardEntry(playerNames[0], playerScores[0]);
        AddLeaderboardEntry(playerNames[1], playerScores[1]);
    }
    else
    {
        AddLeaderboardEntry("Single Player", score + levelScore);
    }

    leaderboardRecorded = true;
}

// ---------- ball physics ----------
 void ResolveCollision(Rectangle obstacle)
{
    if (!CheckCollisionCircleRec(position, BALL_RADIUS, obstacle)) return;

    if (obstacleHitCooldown <= 0.0f && Vector2Length(speed) > 20.0f)
    {
        PlaySound(obsHitl2);
        obstacleHitCooldown = 0.15f;
    }

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

 void ResolveObstacles(void)
{
    if (currentLevel == LEVEL_2)
    {
        ResolveCollision(obstacle_1);
        ResolveCollision(obstacle_2);
        ResolveCollision(obstacle_3);
    }
    else if (currentLevel == LEVEL_5)
    {
        for (int i = 0; i < 3; i++) ResolveCollision(level5Obstacles[i]);
    }
    else if (currentLevel == LEVEL_7)
    {
        for (int i = 0; i < 2; i++) ResolveCollision(level7Walls[i]);
        for (int i = 0; i < 6; i++) ResolveCollision(level7Pillars[i]);
    }
}

void MoveBall(float dt)
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

 void MoveLevel3Ball(float dt)
{
    position.x += speed.x * dt;
    position.y += speed.y * dt;

    if (position.x + BALL_RADIUS > level3Zone1.x && position.x - BALL_RADIUS < level3Zone1.x + level3Zone1.width)
    {
        if (speed.x > 0.0f) speed.x -= GRAYZONE_DECELERATION * dt;
        if (speed.x < 0.0f) speed.x -= GRAYZONE_DECELERATION * dt;
    }

    if (position.x + BALL_RADIUS > level3Zone2.x && position.x - BALL_RADIUS < level3Zone2.x + level3Zone2.width)
    {
        if (speed.x > 0.0f) speed.x += GRAYZONE_DECELERATION * dt;
        if (speed.x < 0.0f) speed.x += GRAYZONE_DECELERATION * dt;
    }
}

 void MoveLevel5Obstacles(float dt)
{
    for (int i = 0; i < 3; i++)
    {
        level5Obstacles[i].y += level5Directions[i] * MOVING_OBS_SPEED * dt;

        if (level5Obstacles[i].y <= STONEWALL)
        {
            level5Obstacles[i].y = STONEWALL;
            level5Directions[i] = 1.0f;
        }
        else if (level5Obstacles[i].y + level5Obstacles[i].height >= HEIGHT - STONEWALL)
        {
            level5Obstacles[i].y = HEIGHT - STONEWALL - level5Obstacles[i].height;
            level5Directions[i] = -1.0f;
        }
    }
}

 bool Level4BallHitPond(void)
{
    return Vector2Distance(position, level4SmallPond) < POND_RADIUS + BALL_RADIUS ||
           CheckCollisionCircleRec(position, BALL_RADIUS, level4LargePond);
}

 bool Level6BallHitSpikes(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (CheckCollisionCircleRec(position, BALL_RADIUS, level6LeftSpikes[i]) ||
            CheckCollisionCircleRec(position, BALL_RADIUS, level6RightSpikes[i]))
        {
            return true;
        }
    }
    return false;
}

 float PipeLength(void)
{
    float length = 0.0f;
    for (int i = 0; i < PIPE_PATH_POINTS - 1; i++) length += Vector2Distance(pipePath[i], pipePath[i + 1]);
    return length;
}

// point on the pipe's middle line, "distance" pixels from the opening the ball went in
 Vector2 PipePoint(float distance)
{
    for (int i = 0; i < PIPE_PATH_POINTS - 1; i++)
    {
        Vector2 from = pipeForward ? pipePath[i] : pipePath[PIPE_PATH_POINTS - 1 - i];
        Vector2 to = pipeForward ? pipePath[i + 1] : pipePath[PIPE_PATH_POINTS - 2 - i];
        float segment = Vector2Distance(from, to);
        if (distance <= segment) return Vector2Lerp(from, to, distance / segment);
        distance -= segment;
    }
    return pipeForward ? pipePath[PIPE_PATH_POINTS - 1] : pipePath[0];
}

// ball enters one pipe mouth, runs along the twisted pipe,
// then comes out of the other mouth (speed capped at PIPE_EXIT_SPEED)
 void UpdateLevel6Pipe(void)
{
    bool inLeftMouth = CheckCollisionCircleRec(position, BALL_RADIUS, level6LeftMouth);
    bool inRightMouth = CheckCollisionCircleRec(position, BALL_RADIUS, level6RightMouth);

    if (canTeleport && (inLeftMouth || inRightMouth))
    {
        inPipe = true;
        pipeForward = inLeftMouth;
        pipeDistance = 0.0f;
        pipeEntrySpeed = Vector2Length(speed);
        speed = (Vector2){0.0f, 0.0f};
        position = PipePoint(0.0f);
        canTeleport = false;
        PlaySound(whoosh);
    }
    else if (!inLeftMouth && !inRightMouth)
    {
        canTeleport = true;
    }
}
 void MoveThroughPipe(float dt)
{
    pipeDistance += PIPE_TRAVEL_SPEED * dt;
    position = PipePoint(pipeDistance);
    if (pipeDistance < PipeLength()) return;

    // comes out at PIPE_EXIT_SPEED, or slower if it went in slower than that
    float exitSpeed = fminf(pipeEntrySpeed, PIPE_EXIT_SPEED);

    if (pipeForward)
    {
        position.x = level6RightMouth.x + level6RightMouth.width - BALL_RADIUS - 1.0f;
        position.y = level6RightMouth.y + level6RightMouth.height / 2.0f;
        speed = (Vector2){exitSpeed, 0.0f};
    }
    else
    {
        position.x = level6LeftMouth.x + BALL_RADIUS + 1.0f;
        position.y = level6LeftMouth.y + level6LeftMouth.height / 2.0f;
        speed = (Vector2){-exitSpeed, 0.0f};
    }
    inPipe = false;
    canTeleport = false;
}
 void UpdateLevel7Portals(void)
{
    for (int i = 0; i < 2; i++)
    {
        if (Vector2Distance(position, portalIn[i]) < PORTAL_RADIUS)
        {
            position = portalOut[i];
            PlaySound(whoosh);
        }
    }
}

 void HitHazard(const char *message, Sound sound)
{
    hitHazard = true;
    hazardMessage = message;
    speed = (Vector2){0.0f, 0.0f};
    PlaySound(sound);
    position = levelStart[currentLevel];
    canTeleport = true;
    inPipe = false;
    textDisplayTimer = 2.0f;
}

// ---------- drawing the courses ----------

 void DrawLevel2Course(void)
{
    Rectangle obstacles[3] = {obstacle_1, obstacle_2, obstacle_3};
    for (int i = 0; i < 3; i++)
    {
        DrawTexturePro(
            pillarL2Texture,
            (Rectangle){0, 0, (float)pillarL2Texture.width, (float)pillarL2Texture.height},
            obstacles[i],
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }
}

 void DrawLevel3Course(void)
{
    DrawRectangleRec(level3Zone1, level3ZoneColor);
    DrawRectangleRec(level3Zone2, level3ZoneColor);
    DrawTexture(arrow1, 200.0f, 0.0f, WHITE);
    DrawTexture(arrow2, 700.0f, 0.0f, WHITE);
}

 void DrawLevel4Course(void)
{
    DrawTexturePro(
        largePond,
        (Rectangle){0, 0, (float)largePond.width, (float)largePond.height},
        (Rectangle){level4LargePond.x, level4LargePond.y, level4LargePond.width, level4LargePond.height},
        (Vector2){0, 0},
        0.0f,
        WHITE);

    DrawTexturePro(
        smallPond,
        (Rectangle){0, 0, (float)smallPond.width, (float)smallPond.height},
        (Rectangle){level4SmallPond.x - POND_RADIUS, level4SmallPond.y - POND_RADIUS, POND_RADIUS * 2.0f, POND_RADIUS * 2.0f},
        (Vector2){0, 0},
        0.0f,
        WHITE);
}

 void DrawLevel5Course(void)
{
    for (int i = 0; i < 3; i++)
    {
        DrawTexturePro(
            movingObsTexture,
            (Rectangle){0, 0, (float)movingObsTexture.width, (float)movingObsTexture.height},
            level5Obstacles[i],
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }
}

 void DrawLevel6Course(void)
{
    if (IsTextureValid(pipeTexture))
    {
        DrawTexturePro(
            pipeTexture,
            (Rectangle){0, 0, (float)pipeTexture.width, (float)pipeTexture.height},
            pipeDrawRect,
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }
    else
    {
        // "Pipe L6.png" is missing, so draw the pipe's path as a thick green line
        for (int i = 0; i < PIPE_PATH_POINTS - 1; i++)
        {
            DrawLineEx(pipePath[i], pipePath[i + 1], 30.0f, darkGreen);
            DrawLineEx(pipePath[i], pipePath[i + 1], 22.0f, (Color){34, 120, 52, 255});
        }
        DrawRectangleRec(level6LeftMouth, Fade(BLACK, 0.55f));
        DrawRectangleRec(level6RightMouth, Fade(BLACK, 0.55f));
    }

    for (int i = 0; i < 4; i++)
    {
        DrawTexturePro(
            spikeLeftTexture,
            (Rectangle){0, 0, (float)spikeLeftTexture.width, (float)spikeLeftTexture.height},
            level6LeftSpikes[i],
            (Vector2){0, 0},
            0.0f,
            LIGHTGRAY);
        DrawTexturePro(
            spikeRightTexture,
            (Rectangle){0, 0, (float)spikeRightTexture.width, (float)spikeRightTexture.height},
            level6RightSpikes[i],
            (Vector2){0, 0},
            0.0f,
            LIGHTGRAY);
    }
}

 void DrawLevel7Course(void)
{
    for (int i = 0; i < 6; i++)
    {
        DrawTexturePro(
            pillarL2Texture,
            (Rectangle){0, 0, (float)pillarL2Texture.width, (float)pillarL2Texture.height},
            level7Pillars[i],
            (Vector2){0, 0},
            0.0f,
            WHITE);
    }

    for (int i = 0; i < 2; i++)
    {
        if (IsTextureValid(portalTexture))
        {
            int frameWidth = portalTexture.width / 5;
            int frameHeight = portalTexture.height / 4;
            Rectangle frameRec = {(float)((portalFrame % 5) * frameWidth), (float)((portalFrame / 5) * frameHeight), (float)frameWidth, (float)frameHeight};
            DrawTexturePro(portalTexture, frameRec,
                (Rectangle){portalIn[i].x - 35.0f, portalIn[i].y - 28.0f, 70.0f, 55.0f},
                (Vector2){0, 0}, 0.0f, WHITE);
            DrawTexturePro(portalTexture, frameRec,
                (Rectangle){portalOut[i].x - 35.0f, portalOut[i].y - 28.0f, 70.0f, 55.0f},
                (Vector2){0, 0}, 0.0f, SKYBLUE);
        }
        else
        {
            // "portals.png" is missing, so draw glowing circles instead
            float pulse = (float)(portalFrame % 5);
            DrawCircleGradient(portalIn[i], PORTAL_RADIUS + 6.0f, Fade(VIOLET, 0.6f), Fade(VIOLET, 0.0f));
            DrawCircleV(portalIn[i], PORTAL_RADIUS, DARKPURPLE);
            DrawCircleV(portalIn[i], PORTAL_RADIUS - 5.0f - pulse, VIOLET);
            DrawCircleGradient(portalOut[i], PORTAL_RADIUS + 6.0f, Fade(SKYBLUE, 0.6f), Fade(SKYBLUE, 0.0f));
            DrawCircleV(portalOut[i], PORTAL_RADIUS, DARKBLUE);
            DrawCircleV(portalOut[i], PORTAL_RADIUS - 5.0f - pulse, SKYBLUE);
        }
    }
}

void DrawHud(void)
{
    int barHeight = gameMode == GAME_MODE_TWO_PLAYER ? HUD_HEIGHT + 28 : HUD_HEIGHT;
    DrawRectangle(0, 0, WIDTH, barHeight, Fade(BLACK, 0.55f));
    DrawRectangle(0, barHeight, WIDTH, 2, Fade(popYellow, 0.8f));

    const char *rightText = TextFormat("Level %d/%d    Shots: %d    ESC: end game", currentLevel + 1, LEVEL_COUNT, shotcount);
    DrawText(rightText, WIDTH - MeasureText(rightText, 20) - 12, 7, 20, RAYWHITE);

    if (gameMode == GAME_MODE_TWO_PLAYER)
    {
        DrawText(TextFormat("%s's turn", playerNames[activePlayer]), 12, 7, 20, popYellow);
        const char *scoresText = TextFormat("%s %d  -  %s %d   (+%d)", playerNames[0], playerScores[0], playerNames[1], playerScores[1], levelScore);
        DrawText(scoresText, 12, HUD_HEIGHT + 2, 18, RAYWHITE);
    }
    else
    {
        DrawText(TextFormat("Score: %d", score + levelScore), 12, 7, 20, popYellow);
    }
}

// ---------- level setup and game flow ----------

void SetupLevel(LevelID level)
{
    currentLevel = level;
    position = levelStart[level];
    hole = levelHole[level];
    speed = (Vector2){0.0f, 0.0f};
    startedplaying = 0;
    hasWon = false;
    hitHazard = false;
    textDisplayTimer = 0.0f;
    shotcount = 0;
    levelScore = 0;
    playerShotCount[0] = 0;
    playerShotCount[1] = 0;
    showNoShotsMessage = false;
    noShotsPlayer = -1;
    noShotsMessageTimer = 0.0f;
    playerFinalBallPosition[0] = (Vector2){0.0f, 0.0f};
    playerFinalBallPosition[1] = (Vector2){0.0f, 0.0f};

    level5Obstacles[0] = (Rectangle){300.0f, 100.0f, MOVING_WIDTH, MOVING_HEIGHT};
    level5Obstacles[1] = (Rectangle){500.0f, 300.0f, MOVING_WIDTH, MOVING_HEIGHT};
    level5Obstacles[2] = (Rectangle){700.0f, 100.0f, MOVING_WIDTH, MOVING_HEIGHT};
    level5Directions[0] = 1.0f;
    level5Directions[1] = -1.0f;
    level5Directions[2] = 1.0f;

    canTeleport = true;
    inPipe = false;
}

void StartGame(LevelID level)
{
    gameOverReason = "";
    activePlayer = 0;
    playerScores[0] = 0;
    playerScores[1] = 0;
    score = 0;
    playerFinishedCurrentLevel[0] = false;
    playerFinishedCurrentLevel[1] = false;
    playerExhausted[0] = false;
    playerExhausted[1] = false;
    leaderboardRecorded = false;
    SetupLevel(level);
    StopMusicStream(startingMenu);
    PlayMusicStream(levelMusic);
    state = STATE_PLAYING;
}

void GoToMenu(void)
{
    StopMusicStream(levelMusic);
    PlayMusicStream(startingMenu);
    showTwoPlayerNameEntry = false;
    state = STATE_MENU;
}

void EndGameEarly(void)
{
    int displayedScore = score + levelScore;
    if (displayedScore > highScore)
    {
        highScore = displayedScore;
        SaveHighScore();
    }
    state = STATE_GAMEOVER;
}

// two player name entry layout (used by update and draw so they always match)
Rectangle name1Box = {400.0f, 205.0f, 340.0f, 50.0f};
Rectangle name2Box = {400.0f, 280.0f, 340.0f, 50.0f};
Rectangle startMatchButton = {340.0f, 365.0f, 320.0f, 56.0f};
Rectangle nameBackButton = {390.0f, 440.0f, 220.0f, 44.0f};
void TypeIntoName(char *name)
{
    int c = GetCharPressed();
    while (c > 0)
    {
        if (c >= 32 && c <= 126 && c != '|' && (int)strlen(name) < MAX_PLAYER_NAME_LENGTH - 1)
        {
            char ch[2] = {(char)c, '\0'};
            strcat(name, ch);
        }
        c = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(name) > 0)
    {
        name[strlen(name) - 1] = '\0';
    }
}

 void ConfirmPlayerNames(void)
{
    if (playerNames[0][0] == '\0') snprintf(playerNames[0], MAX_PLAYER_NAME_LENGTH, "Player 1");
    if (playerNames[1][0] == '\0') snprintf(playerNames[1], MAX_PLAYER_NAME_LENGTH, "Player 2");
    gameMode = GAME_MODE_TWO_PLAYER;
    showTwoPlayerNameEntry = false;
    state = STATE_LEVEL_SELECT;
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetExitKey(KEY_NULL); // ESC ends the game instead of closing the window
    SetTargetFPS(60);

    Font minecrafterFont = LoadFontEx("Assets/Fonts/Minecrafter-MA3Dw.ttf", 96, NULL, 0);
    SetTextureFilter(minecrafterFont.texture, TEXTURE_FILTER_BILINEAR);
    uiFont = minecrafterFont;

    Font minecrafterAltFont = LoadFontEx("Assets/Fonts/Minecrafter-MA3Dw.ttf", 48, NULL, 0);
    SetTextureFilter(minecrafterAltFont.texture, TEXTURE_FILTER_BILINEAR);
    myFont = minecrafterAltFont;

    levelBackgroundTexture = LoadTexture("Assets/Textures/Level background.png");
    level5Background = LoadTexture("Assets/Textures/level 5 background.png");
    level6Background = LoadTexture("Assets/Textures/Background L6.png");
    level7Background = LoadTexture("Assets/Textures/Background L7.png");
    menuTexture = LoadTexture("Assets/Textures/startmenubackground.png");
    buttonTexture = LoadTexture("Assets/Textures/Menu Options.png");
    SetTextureFilter(buttonTexture, TEXTURE_FILTER_BILINEAR);
    arrow1 = LoadTexture("Assets/Textures/arrow zone 1.png");
    arrow2 = LoadTexture("Assets/Textures/arrow zone 2.png");
    congratulationsTexture = LoadTexture("Assets/Textures/Congratulations.png");
    SetTextureFilter(congratulationsTexture, TEXTURE_FILTER_BILINEAR);
    pillarL2Texture = LoadTexture("Assets/Textures/pillar L2.png");
    largePond = LoadTexture("Assets/Textures/Large pond.png");
    smallPond = LoadTexture("Assets/Textures/Small pond.png");
    GameOver = LoadTexture("Assets/Textures/Game over.png");
    movingObsTexture = LoadTexture("Assets/Textures/Moving obstacles.png");
    spikeLeftTexture = LoadTexture("Assets/Textures/spikes left.png");
    spikeRightTexture = LoadTexture("Assets/Textures/spikes right.png");
    pipeTexture = LoadTexture("Assets/Textures/Pipe L6.png");
    portalTexture = LoadTexture("Assets/Textures/portals.png");

    LoadHighScore();
    LoadLeaderboard();

    InitAudioDevice();
    startingMenu = LoadMusicStream("Assets/Audio/Starting music.mp3");
    levelMusic = LoadMusicStream("Assets/Audio/Level Music.mp3");
    ballHitSound = LoadSound("Assets/Audio/Ball Hit.mp3");
    ballPuttSound = LoadSound("Assets/Audio/Ball Putt.mp3");
    levelCompletion = LoadSound("Assets/Audio/Level Completion.mp3");
    levelFailed = LoadSound("Assets/Audio/Level Failed.mp3");
    waterSplash = LoadSound("Assets/Audio/Water splash.mp3");
    wallHit = LoadSound("Assets/Audio/ball hit wall.mp3");
    obsHitl2 = LoadSound("Assets/Audio/ball hit obs.wav");
    spikeHit = LoadSound("Assets/Audio/Ball Hit Spike.mp3");
    whoosh = LoadSound("Assets/Audio/pipe_whoosh.mp3");
    PlayMusicStream(startingMenu);

    while (!WindowShouldClose() && !quitRequested)
    {
        UpdateMusicStream(startingMenu);
        UpdateMusicStream(levelMusic);
        float dt = GetFrameTime();
        obstacleHitCooldown = fmaxf(0.0f, obstacleHitCooldown - dt);
        anyButtonHovered = false;

        switch (state)
        {
            case STATE_MENU:
            {
                if (showTwoPlayerNameEntry)
                {
                    Vector2 mousePos = GetMousePosition();

                    if (CheckCollisionPointRec(mousePos, name1Box) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        activePlayerNameField = 0;
                    }
                    if (CheckCollisionPointRec(mousePos, name2Box) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        activePlayerNameField = 1;
                    }
                    if (IsKeyPressed(KEY_TAB))
                    {
                        activePlayerNameField = 1 - activePlayerNameField;
                    }

                    TypeIntoName(playerNames[activePlayerNameField]);

                    if (IsKeyPressed(KEY_ENTER))
                    {
                        ConfirmPlayerNames();
                    }
                }
                break;
            }

            case STATE_LEVEL_SELECT:
            {
                for (int i = 0; i < LEVEL_COUNT; i++)
                {
                    if (IsKeyPressed(KEY_ONE + i)) StartGame((LevelID)i);
                }
                if (IsKeyPressed(KEY_BACKSPACE)) state = STATE_MENU;
                break;
            }

            case STATE_PLAYING:
            {
                if (currentLevel == LEVEL_5)
                {
                    MoveLevel5Obstacles(dt);
                }

                if (currentLevel == LEVEL_6 && inPipe)
                {
                    MoveThroughPipe(dt);
                }
                else if (currentLevel == LEVEL_3)
                {
                    MoveLevel3Ball(dt);
                }
                else
                {
                    MoveBall(dt);
                }

                // outer walls: the screen edge, or the stone border drawn in the background
                float borderX = 0.0f;
                float borderY = 0.0f;
                if (currentLevel == LEVEL_5)
                {
                    borderX = STONEWALL;
                    borderY = STONEWALL;
                }
                else if (currentLevel == LEVEL_7)
                {
                    borderX = 20.0f;
                    borderY = 18.0f;
                }

                if (position.y - BALL_RADIUS < borderY)
                {
                    PlaySound(wallHit);
                    position.y = borderY + BALL_RADIUS;
                    speed.y = -speed.y;
                }
                else if (position.y + BALL_RADIUS > HEIGHT - borderY)
                {
                    PlaySound(wallHit);
                    position.y = HEIGHT - borderY - BALL_RADIUS;
                    speed.y = -speed.y;
                }

                if (position.x - BALL_RADIUS < borderX)
                {
                    PlaySound(wallHit);
                    position.x = borderX + BALL_RADIUS;
                    speed.x = -speed.x;
                }
                else if (position.x + BALL_RADIUS > WIDTH - borderX)
                {
                    PlaySound(wallHit);
                    position.x = WIDTH - borderX - BALL_RADIUS;
                    speed.x = -speed.x;
                }

                ResolveObstacles();

                if (currentLevel == LEVEL_4 && !hasWon && !hitHazard && Level4BallHitPond())
                {
                    HitHazard("YOU HIT THE POND", waterSplash);
                }

                if (currentLevel == LEVEL_6 && !inPipe && !hasWon && !hitHazard)
                {
                    if (Level6BallHitSpikes())
                    {
                        // spikes end the whole game straight away
                        PlaySound(spikeHit);
                        PlaySound(levelFailed);
                        speed = (Vector2){0.0f, 0.0f};
                        gameOverReason = "YOU HIT THE SPIKES";
                        EndGameEarly();
                    }
                    else
                    {
                        UpdateLevel6Pipe();
                    }
                }

                if (currentLevel == LEVEL_7 && !hasWon && !hitHazard)
                {
                    UpdateLevel7Portals();
                }

                portalFrameTimer += dt;
                if (portalFrameTimer >= 0.08f)
                {
                    portalFrameTimer = 0.0f;
                    portalFrame = (portalFrame + 1) % 20;
                }

                if (IsKeyPressed(KEY_ESCAPE))
                {
                    EndGameEarly();
                }

                if (!showNoShotsMessage && !playerExhausted[activePlayer] && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && Vector2Length(speed) < STOP_THRESHOLD && startedplaying == 0 && !hitHazard && !inPipe)
                {
                    startedplaying = 1;
                    shotcount++;
                    playerShotCount[activePlayer] = shotcount;
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && startedplaying == 1)
                {
                    mouse = GetMousePosition();
                }

                if (shotcount == 1 && startedplaying == 1) levelScore = 25;
                else if (shotcount == 2 && startedplaying == 1) levelScore = 20;
                else if (shotcount == 3 && startedplaying == 1) levelScore = 17;
                else if (shotcount == 4 && startedplaying == 1) levelScore = 14;
                else if (shotcount == 5 && startedplaying == 1) levelScore = 11;
                else if (shotcount > 5 && startedplaying == 1) levelScore = 0;

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && startedplaying == 1)
                {
                    mouse = GetMousePosition();
                    Vector2 shotdir = { position.x - mouse.x, position.y - mouse.y };
                    float dragDistance = Vector2Length(shotdir);
                    if (3.0f * dragDistance > MAX_SPEED) dragDistance = MAX_SPEED / 3.0f;
                    if (dragDistance > 0.0f)
                    {
                        shotdir = Vector2Normalize(shotdir);
                        speed.x = 9.0f * shotdir.x * dragDistance;
                        speed.y = 9.0f * shotdir.y * dragDistance;
                        PlaySound(ballHitSound);
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

                if (gameMode == GAME_MODE_SINGLE && shotcount > 5 && Vector2Length(speed) < STOP_THRESHOLD && !hasWon && !hitHazard && !inPipe)
                {
                    state = STATE_GAMEOVER;
                    textDisplayTimer = 0.0f;
                }
                else if (gameMode == GAME_MODE_TWO_PLAYER && shotcount >= 5 && Vector2Length(speed) < STOP_THRESHOLD && !hasWon && !hitHazard && !inPipe && !showNoShotsMessage)
                {
                    noShotsPlayer = activePlayer;
                    playerExhausted[activePlayer] = true;
                    showNoShotsMessage = true;
                    noShotsMessageTimer = 1.8f;
                    shotcount = 0;
                    playerShotCount[activePlayer] = 5;
                    startedplaying = 0;
                    speed = (Vector2){0.0f, 0.0f};
                }

                Vector2 diffHoleBall = { position.x - hole.x, position.y - hole.y };
                if (!hasWon && !hitHazard && !inPipe && Vector2Length(diffHoleBall) < 10.0f)
                {
                    hasWon = true;
                    speed = (Vector2){0.0f, 0.0f};
                    PlaySound(ballPuttSound);
                    PlaySound(levelCompletion);
                    position = levelStart[currentLevel];
                    textDisplayTimer = 2.0f;
                }

                if (textDisplayTimer > 0.0f)
                {
                    textDisplayTimer -= dt;
                }

                if (showNoShotsMessage)
                {
                    noShotsMessageTimer -= dt;
                    if (noShotsMessageTimer <= 0.0f)
                    {
                        showNoShotsMessage = false;
                        noShotsPlayer = -1;
                        if (playerExhausted[0] && playerExhausted[1])
                        {
                            state = STATE_GAMEOVER;
                        }
                        else if (playerFinishedCurrentLevel[0] || playerFinishedCurrentLevel[1])
                        {
                            if (currentLevel == LEVEL_7)
                            {
                                state = STATE_GAMEOVER;
                            }
                            else
                            {
                                currentLevel = (LevelID)((int)currentLevel + 1);
                                activePlayer = 0;
                                playerFinishedCurrentLevel[0] = false;
                                playerFinishedCurrentLevel[1] = false;
                                playerExhausted[0] = false;
                                playerExhausted[1] = false;
                                SetupLevel(currentLevel);
                                state = STATE_PLAYING;
                            }
                        }
                        else
                        {
                            activePlayer = playerExhausted[0] ? 1 : 0;
                            SetupLevel(currentLevel);
                            startedplaying = 0;
                            speed = (Vector2){0.0f, 0.0f};
                        }
                    }
                }

                if (textDisplayTimer <= 0.0f && hasWon)
                {
                    if (gameMode == GAME_MODE_TWO_PLAYER)
                    {
                        playerFinalBallPosition[activePlayer] = position;
                        playerScores[activePlayer] += levelScore;
                        playerFinishedCurrentLevel[activePlayer] = true;
                        score = playerScores[activePlayer];
                        levelScore = 0;

                        if (activePlayer == 0 && !playerExhausted[1])
                        {
                            activePlayer = 1;
                            SetupLevel(currentLevel);
                            state = STATE_PLAYING;
                        }
                        else if ((playerFinishedCurrentLevel[0] && playerFinishedCurrentLevel[1]) || playerExhausted[0] || playerExhausted[1])
                        {
                            if (currentLevel == LEVEL_7)
                            {
                                int winner = playerScores[0] >= playerScores[1] ? 0 : 1;
                                score = playerScores[winner];
                                if (score > highScore)
                                {
                                    highScore = score;
                                    SaveHighScore();
                                }
                                state = STATE_GAMEOVER;
                            }
                            else
                            {
                                currentLevel = (LevelID)((int)currentLevel + 1);
                                activePlayer = 0;
                                playerFinishedCurrentLevel[0] = false;
                                playerFinishedCurrentLevel[1] = false;
                                playerExhausted[0] = false;
                                playerExhausted[1] = false;
                                SetupLevel(currentLevel);
                                state = STATE_PLAYING;
                            }
                        }
                        else
                        {
                            activePlayer = 0;
                            SetupLevel(currentLevel);
                            state = STATE_PLAYING;
                        }
                    }
                    else
                    {
                        score += levelScore;
                        levelScore = 0;

                        if (score > highScore)
                        {
                            highScore = score;
                            SaveHighScore();
                        }

                        if (currentLevel == LEVEL_7)
                        {
                            state = STATE_GAMEOVER;
                        }
                        else
                        {
                            SetupLevel((LevelID)((int)currentLevel + 1));
                            state = STATE_PLAYING;
                        }
                    }

                    hasWon = false;
                }

                if (textDisplayTimer <= 0.0f && hitHazard)
                {
                    hitHazard = false;
                }

                break;
            }

            case STATE_LEVEL_FAILED:
                if(!levelFailedSoundPlayed)
                {
                    PlaySound(levelFailed);
                    levelFailedSoundPlayed = true;
                }
                if (IsKeyPressed(KEY_R))
                {
                    SetupLevel(currentLevel);
                    state = STATE_PLAYING;
                }
                if (IsKeyPressed(KEY_Q))
                {
                    EndGameEarly();
                }
                break;

            case STATE_GAMEOVER:
                RecordCurrentGameOnLeaderboard();
                if (IsKeyPressed(KEY_ENTER)) GoToMenu();
                else if (IsKeyPressed(KEY_R)) StartGame(currentLevel);
                break;

            case STATE_HIGHSCORE:
                if (IsKeyPressed(KEY_BACKSPACE)) state = STATE_MENU;
                break;

            case STATE_LEADERBOARD:
                if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) state = STATE_MENU;
                break;

            case STATE_ACKNOWLEDGEMENT:
                if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) state = STATE_MENU;
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        switch (state)
        {
            case STATE_MENU:
            {
                DrawFullScreen(menuTexture);

                if (!showTwoPlayerNameEntry)
                {
                    // buttons sit under the Golf Mania logo
                    float buttonX = 494.0f;
                    float buttonWidth = 320.0f;

                    if (ButtonPressed((Rectangle){buttonX, 335, buttonWidth, 44}, "SINGLE PLAYER", 24))
                    {
                        gameMode = GAME_MODE_SINGLE;
                        state = STATE_LEVEL_SELECT;
                    }

                    if (ButtonPressed((Rectangle){buttonX, 385, buttonWidth, 44}, "TWO PLAYERS", 24))
                    {
                        activePlayerNameField = 0;
                        showTwoPlayerNameEntry = true;
                        playerNames[0][0] = '\0';
                        playerNames[1][0] = '\0';
                    }

                    if (ButtonPressed((Rectangle){buttonX, 435, buttonWidth, 44}, "LEADERBOARD", 24))
                    {
                        state = STATE_LEADERBOARD;
                    }

                    if (ButtonPressed((Rectangle){buttonX, 485, buttonWidth, 44}, "ACKNOWLEDGEMENTS", 24))
                    {
                        state = STATE_ACKNOWLEDGEMENT;
                    }

                    if (ButtonPressed((Rectangle){buttonX, 535, buttonWidth, 44}, "QUIT", 24))
                    {
                        quitRequested = true;
                    }
                }
                else
                {
                    nameEntryBlinkTimer += GetFrameTime();
                    DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.5f));

                    Rectangle panel = {220.0f, 95.0f, 560.0f, 420.0f};
                    DrawRectangleRounded(panel, 0.08f, 10, panelColor);
                    DrawRectangleRoundedLinesEx(panel, 0.08f, 10, 4.0f, popYellow);
                    DrawBannerText("ENTER PLAYER NAMES", 118.0f, 40.0f, popGreen, darkGreen);

                    DrawTextEx(uiFont, "PLAYER 1", (Vector2){250.0f, name1Box.y + 12.0f}, 26, 2, RAYWHITE);
                    DrawTextEx(uiFont, "PLAYER 2", (Vector2){250.0f, name2Box.y + 12.0f}, 26, 2, RAYWHITE);

                    DrawRectangleRounded(name1Box, 0.25f, 8, RAYWHITE);
                    DrawRectangleRounded(name2Box, 0.25f, 8, RAYWHITE);
                    DrawRectangleRoundedLinesEx(activePlayerNameField == 0 ? name1Box : name2Box, 0.25f, 8, 4.0f, popYellow);
                    DrawText(playerNames[0], (int)name1Box.x + 15, (int)name1Box.y + 13, 24, BLACK);
                    DrawText(playerNames[1], (int)name2Box.x + 15, (int)name2Box.y + 13, 24, BLACK);

                    if ((int)(nameEntryBlinkTimer * 2.0f) % 2 == 0)
                    {
                        Rectangle activeBox = activePlayerNameField == 0 ? name1Box : name2Box;
                        int xBase = (int)activeBox.x + 15 + MeasureText(playerNames[activePlayerNameField], 24) + 2;
                        DrawRectangle(xBase, (int)activeBox.y + 12, 3, 26, BLACK);
                    }

                    DrawCenteredText("TAB switches name, ENTER starts", 340, 18, LIGHTGRAY);

                    if (ButtonPressed(startMatchButton, "START MATCH", 28))
                    {
                        ConfirmPlayerNames();
                    }
                    if (ButtonPressed(nameBackButton, "BACK", 22))
                    {
                        showTwoPlayerNameEntry = false;
                    }
                }
                break;
            }

            case STATE_LEVEL_SELECT:
            {
                DrawFullScreen(menuTexture);
                DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.55f));

                DrawBannerText("SELECT LEVEL", 30.0f, 64.0f, popGreen, darkGreen);
                if (gameMode == GAME_MODE_TWO_PLAYER)
                {
                    DrawCenteredText(TextFormat("%s  vs  %s", playerNames[0], playerNames[1]), 112, 22, popYellow);
                }
                else
                {
                    DrawCenteredText("Single Player", 112, 22, popYellow);
                }

                // 4 tiles on the first row, 3 on the second
                float tileWidth = 200.0f;
                float previewHeight = 110.0f;
                float labelHeight = 40.0f;
                float gap = 30.0f;

                for (int i = 0; i < LEVEL_COUNT; i++)
                {
                    int row = i < 4 ? 0 : 1;
                    int column = i < 4 ? i : i - 4;
                    int tilesInRow = row == 0 ? 4 : 3;
                    float rowWidth = tilesInRow * tileWidth + (tilesInRow - 1) * gap;
                    float x = (WIDTH - rowWidth) / 2.0f + column * (tileWidth + gap);
                    float y = 155.0f + row * (previewHeight + labelHeight + 35.0f);

                    Rectangle tile = {x, y, tileWidth, previewHeight + labelHeight};
                    bool hovered = CheckCollisionPointRec(GetMousePosition(), tile);
                    if (hovered)
                    {
                        anyButtonHovered = true;
                        tile.y -= 4.0f;
                    }

                    Rectangle preview = {tile.x, tile.y, tileWidth, previewHeight};
                    Rectangle label = {tile.x, tile.y + previewHeight, tileWidth, labelHeight};
                    float scaleX = tileWidth / WIDTH;
                    float scaleY = previewHeight / HEIGHT;

                    DrawRectangle((int)x + 5, (int)y + 7, (int)tileWidth, (int)(previewHeight + labelHeight), Fade(BLACK, 0.5f));

                    Texture2D background = LevelBackground((LevelID)i);
                    DrawTexturePro(background,
                        (Rectangle){0, 0, (float)background.width, (float)background.height},
                        preview, (Vector2){0, 0}, 0.0f, WHITE);

                    // little start ball and hole so every tile looks different
                    Vector2 previewHole = {preview.x + levelHole[i].x * scaleX, preview.y + levelHole[i].y * scaleY};
                    Vector2 previewBall = {preview.x + levelStart[i].x * scaleX, preview.y + levelStart[i].y * scaleY};
                    DrawCircleV(previewHole, 6.0f, BLACK);
                    DrawCircleV(previewBall, 5.0f, BLACK);
                    DrawCircleV(previewBall, 4.0f, WHITE);

                    DrawTexturePro(buttonTexture, buttonSource, label, (Vector2){0, 0}, 0.0f, hovered ? (Color){255, 238, 190, 255} : WHITE);
                    const char *levelText = TextFormat("LEVEL %d", i + 1);
                    Vector2 textSize = MeasureTextEx(uiFont, levelText, 24, 2);
                    Vector2 textPos = {label.x + (label.width - textSize.x) / 2.0f, label.y + (label.height - textSize.y) / 2.0f};
                    DrawTextEx(uiFont, levelText, (Vector2){textPos.x + 2, textPos.y + 2}, 24, 2, Fade(BLACK, 0.75f));
                    DrawTextEx(uiFont, levelText, textPos, 24, 2, hovered ? popYellow : RAYWHITE);

                    DrawRectangleLinesEx(tile, hovered ? 4.0f : 2.0f, hovered ? popYellow : Fade(RAYWHITE, 0.6f));

                    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    {
                        StartGame((LevelID)i);
                    }
                }

                if (ButtonPressed((Rectangle){400, 530, 200, 44}, "BACK", 22))
                {
                    state = STATE_MENU;
                }
                break;
            }

            case STATE_PLAYING:
            {
                DrawFullScreen(LevelBackground(currentLevel));

                if (currentLevel == LEVEL_2) DrawLevel2Course();
                else if (currentLevel == LEVEL_3) DrawLevel3Course();
                else if (currentLevel == LEVEL_4) DrawLevel4Course();
                else if (currentLevel == LEVEL_5) DrawLevel5Course();
                else if (currentLevel == LEVEL_6) DrawLevel6Course();
                else if (currentLevel == LEVEL_7) DrawLevel7Course();

                DrawCircleV(hole, HOLE_RADIUS, BLACK);

                if (!hasWon)
                {
                    DrawCircle(position.x, position.y, BALL_RADIUS + 2.0f, BLACK);
                    DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
                }

                if (startedplaying == 1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    mouse = GetMousePosition();

                    Vector2 shotdir = { position.x - mouse.x, position.y - mouse.y };
                    float dragDistance = Vector2Length(shotdir);
                    if (3.0f * dragDistance > MAX_SPEED) dragDistance = MAX_SPEED / 3.0f;

                    if (dragDistance > 0.0f)
                    {
                        shotdir = Vector2Normalize(shotdir);
                        int circleCount = 8;
                        float circleRadius = 4.0f;
                        float spacing = (dragDistance * 3.0f) / circleCount;

                        for (int i = 1; i <= circleCount; i++)
                        {
                            Vector2 circlePos = {
                                position.x + shotdir.x * spacing * i,
                                position.y + shotdir.y * spacing * i
                            };
                            DrawCircleV(circlePos, circleRadius, BLACK);
                        }
                    }
                }

                DrawHud();

                // every message on every level uses the same banner look
                if (textDisplayTimer > 0.0f)
                {
                    DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.3f));
                    if (hitHazard)
                    {
                        DrawBannerText("OH NO", 110.0f, 90.0f, popRed, darkRed);
                        DrawBannerText(hazardMessage, 215.0f, 46.0f, popRed, darkRed);
                    }
                    else
                    {
                        float scale = 0.4f;
                        Vector2 drawPosition = {
                            (WIDTH - congratulationsTexture.width * scale) / 2.0f,
                            70.0f
                        };
                        DrawTextureEx(congratulationsTexture, drawPosition, 0.0f, scale, WHITE);
                        DrawBannerText(TextFormat("%d POINTS", levelScore), 300.0f, 44.0f, popGreen, darkGreen);
                    }
                }

                if (showNoShotsMessage && noShotsPlayer >= 0)
                {
                    DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.3f));
                    DrawBannerText("OUT OF SHOTS", 150.0f, 70.0f, popOrange, darkOrange);
                    DrawCenteredText(TextFormat("%s used all 5 shots", playerNames[noShotsPlayer]), 250, 26, RAYWHITE);
                    DrawCenteredText("Next player will continue...", 290, 20, LIGHTGRAY);
                }

                break;
            }

            case STATE_LEVEL_FAILED:
            {
                DrawFullScreen(GameOver);
                DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.4f));
                DrawBannerText("LEVEL FAILED", 120.0f, 70.0f, popRed, darkRed);
                DrawCenteredText("You exceeded 5 shots!", 240, 24, RAYWHITE);
                DrawCenteredText("Press R to restart the level", 320, 20, popYellow);
                DrawCenteredText("Press Q to end the game", 350, 20, popYellow);
                break;
            }

            case STATE_HIGHSCORE:
            {
                DrawFullScreen(menuTexture);
                DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.6f));
                DrawBannerText(TextFormat("HIGH SCORE %d", highScore), 200.0f, 56.0f, popGreen, darkGreen);
                DrawCenteredText("Press BACKSPACE to go back", 350, 20, LIGHTGRAY);
                break;
            }

            case STATE_LEADERBOARD:
            {
                DrawFullScreen(menuTexture);
                DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.55f));

                DrawBannerText("LEADERBOARD", 25.0f, 60.0f, popGreen, darkGreen);

                Rectangle panel = {180.0f, 105.0f, 640.0f, 400.0f};
                DrawRectangleRounded(panel, 0.06f, 10, panelColor);
                DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 4.0f, popYellow);

                DrawTextEx(myFont, "RANK", (Vector2){215, 120}, 22, 1.0f, popYellow);
                DrawTextEx(myFont, "PLAYER", (Vector2){340, 120}, 22, 1.0f, popYellow);
                DrawTextEx(myFont, "POINTS", (Vector2){660, 120}, 22, 1.0f, popYellow);

                Color rankColors[3] = {popYellow, (Color){200, 210, 220, 255}, (Color){215, 140, 70, 255}};
                for (int i = 0; i < leaderboardCount; i++)
                {
                    int y = 158 + i * 33;
                    if (i % 2 == 0) DrawRectangle(196, y - 5, 608, 31, Fade(WHITE, 0.07f));
                    Color rowColor = i < 3 ? rankColors[i] : RAYWHITE;
                    DrawTextEx(myFont, TextFormat("%d", i + 1), (Vector2){235, y}, 22, 1.0f, rowColor);
                    DrawTextEx(myFont, leaderboard[i].name, (Vector2){340, y}, 22, 1.0f, rowColor);
                    DrawTextEx(myFont, TextFormat("%d", leaderboard[i].points), (Vector2){680, y}, 22, 1.0f, rowColor);
                }

                if (leaderboardCount == 0)
                {
                    DrawCenteredText("No scores recorded yet.", 280, 22, RAYWHITE);
                }

                if (ButtonPressed((Rectangle){400, 525, 200, 44}, "BACK", 22))
                {
                    state = STATE_MENU;
                }
                break;
            }

            case STATE_ACKNOWLEDGEMENT:
            {
                DrawFullScreen(menuTexture);
                DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.55f));

                DrawBannerText("ACKNOWLEDGEMENTS", 50.0f, 52.0f, popGreen, darkGreen);

                Rectangle panel = {60.0f, 150.0f, 880.0f, 300.0f};
                DrawRectangleRounded(panel, 0.06f, 10, panelColor);
                DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 4.0f, popYellow);

                DrawCenteredText("Golf Mania was developed as part of our CSE102 project.", 185, 20, RAYWHITE);
                DrawCenteredText("Project Members: Mahian/2505158 and Rafsan/2505155", 235, 20, popYellow);
                DrawCenteredText("Built using the raylib library.", 295, 18, RAYWHITE);
                DrawCenteredText("We thank the raylib community for their tools and documentation.", 322, 18, RAYWHITE);
                DrawCenteredText("Special thanks to Sumaiya Sultana Any ma'am", 375, 18, RAYWHITE);
                DrawCenteredText("for her valuable guidance and support throughout this project.", 402, 18, RAYWHITE);

                if (ButtonPressed((Rectangle){400, 490, 200, 44}, "BACK", 22))
                {
                    state = STATE_MENU;
                }
                break;
            }

            case STATE_GAMEOVER:
            {
                DrawFullScreen(GameOver);

                if (gameOverReason[0] != '\0')
                {
                    DrawBannerText(gameOverReason, 40.0f, 44.0f, popRed, darkRed);
                }

                Rectangle panel = {250.0f, 425.0f, 500.0f, 160.0f};
                DrawRectangleRounded(panel, 0.12f, 10, panelColor);
                DrawRectangleRoundedLinesEx(panel, 0.12f, 10, 4.0f, popYellow);

                if (gameMode == GAME_MODE_TWO_PLAYER)
                {
                    if (playerScores[0] == playerScores[1])
                    {
                        DrawBannerText("DRAW", 432.0f, 40.0f, popOrange, darkOrange);
                    }
                    else
                    {
                        int winner = playerScores[0] > playerScores[1] ? 0 : 1;
                        DrawCenteredText(TextFormat("Winner: %s", playerNames[winner]), 440, 30, popYellow);
                    }
                    DrawCenteredText(TextFormat("%s: %d     %s: %d", playerNames[0], playerScores[0], playerNames[1], playerScores[1]), 485, 22, RAYWHITE);
                }
                else
                {
                    DrawBannerText(TextFormat("SCORE %d", score + levelScore), 432.0f, 44.0f, popGreen, darkGreen);
                    DrawCenteredText(TextFormat("High score: %d", highScore), 490, 20, popYellow);
                }
                shotcount = 0;

                if (ButtonPressed((Rectangle){270, 522, 220, 44}, "MAIN MENU", 22))
                {
                    GoToMenu();
                }
                else if (ButtonPressed((Rectangle){510, 522, 220, 44}, "RESTART LEVEL", 22))
                {
                    // new game on the same level, same mode and same player names
                    StartGame(currentLevel);
                }
                break;
            }
        }

        SetMouseCursor(anyButtonHovered ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
        EndDrawing();
    }

    UnloadFont(uiFont);
    UnloadFont(myFont);
    UnloadTexture(levelBackgroundTexture);
    UnloadTexture(level5Background);
    UnloadTexture(level6Background);
    UnloadTexture(level7Background);
    UnloadTexture(menuTexture);
    UnloadTexture(buttonTexture);
    UnloadTexture(arrow1);
    UnloadTexture(arrow2);
    UnloadTexture(congratulationsTexture);
    UnloadTexture(pillarL2Texture);
    UnloadTexture(largePond);
    UnloadTexture(smallPond);
    UnloadTexture(GameOver);
    UnloadTexture(movingObsTexture);
    UnloadTexture(spikeLeftTexture);
    UnloadTexture(spikeRightTexture);
    UnloadTexture(pipeTexture);
    UnloadTexture(portalTexture);
    UnloadMusicStream(startingMenu);
    UnloadMusicStream(levelMusic);
    UnloadSound(ballHitSound);
    UnloadSound(ballPuttSound);
    UnloadSound(levelCompletion);
    UnloadSound(levelFailed);
    UnloadSound(waterSplash);
    UnloadSound(wallHit);
    UnloadSound(obsHitl2);
    UnloadSound(spikeHit);
    UnloadSound(whoosh);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
