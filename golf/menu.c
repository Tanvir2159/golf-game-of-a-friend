#include "raylib.h"
#include <stdlib.h> // Required for atoi()



typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_HIGHSCORE,
    STATE_GAMEOVER
} GameState;

int highScore = 0;
int score = 0;

bool ButtonPressed(Rectangle bounds, const char* text, float fontSize) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRec(bounds, hovered ? LIGHTGRAY : GRAY);
    DrawText(text, bounds.x + 20, bounds.y + 10, fontSize, BLACK);
        return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void LoadHighScore(void) {
    char *text = LoadFileText("highscore.txt");
    if (text != NULL) {
        highScore = atoi(text);
        UnloadFileText(text); // Free memory allocated by Raylib
    }
}

void SaveHighScore(void) {
    SaveFileText("highscore.txt", (char *)TextFormat("%d", highScore));
}

int main(void) {
    InitWindow(800, 600, "My Game");
    SetTargetFPS(60);
    LoadHighScore();

    GameState state = STATE_MENU;

    while (!WindowShouldClose()) {
        // ---- UPDATE ----
        switch (state) {
            case STATE_MENU:
                break; // Handled in draw section via buttons
            case STATE_PLAYING:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    if (score > highScore) { 
                        highScore = score; 
                        SaveHighScore(); 
                    }
                    state = STATE_GAMEOVER;
                }
                // ... your game logic, update score, etc.
                break;
            case STATE_GAMEOVER:
                if (IsKeyPressed(KEY_ENTER)) state = STATE_MENU;
                break;
            case STATE_HIGHSCORE:
                if (IsKeyPressed(KEY_BACKSPACE)) state = STATE_MENU;
                break;
        }

        // ---- DRAW ----
        BeginDrawing();
      //ClearBackground(RAYWHITE);

        switch (state) {
            case STATE_MENU: {
                  ClearBackground(RAYWHITE);

                DrawText("MY GAME", 300, 100, 40, DARKBLUE);
                if (ButtonPressed((Rectangle){300, 220, 200, 50}, "Start", 20)) { 
                    score = 0; 
                    state = STATE_PLAYING; 
                }
                if (ButtonPressed((Rectangle){300, 290, 200, 50}, "High Score", 20)) {
                    state = STATE_HIGHSCORE;
                }
                if (ButtonPressed((Rectangle){300, 360, 200, 50}, "Quit", 20)) {
                    CloseWindow();
                }
                break;
            }
            case STATE_PLAYING:{
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
                DrawText("Press ESC to end", 10, 40, 20, GRAY);
                // ... draw your game
                break;
            case STATE_HIGHSCORE:
                DrawText(TextFormat("High Score: %d", highScore), 280, 250, 30, MAROON);
                DrawText("Press BACKSPACE to go back", 220, 320, 20, GRAY);
                break;
            case STATE_GAMEOVER:
                DrawText(TextFormat("Game Over! Score: %d", score), 200, 250, 30, RED);
                DrawText("Press ENTER for menu", 250, 320, 20, GRAY);
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}