#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h> //  atoi er jonno

#define WIDTH 1000
#define HEIGHT 600 
#define BALL_SPEED 500
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 1000.0f  // Maximum speed based on drag distance
#define DECELARATION 100.0f

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_HIGHSCORE,
} GameState;

int highScore = 0;
int score = 0;
  static bool hasWon = false;
  int startedplaying = 0;
  int shotcount = 0;

Vector2 position = {(WIDTH*1.0)/10.0f, HEIGHT/2.0f}; //ini pos of ball
Vector2 speed = {0.0f, 0.0f}; //initially at rest
Vector2 mouse;
Vector2 hole = {(WIDTH*9.0)/10.0f, HEIGHT/2.0f};

float textDisplayTimer = 0.0f;

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
        UnloadFileText(text); 
    }
}

void SaveHighScore(void) {
    SaveFileText("highscore.txt", (char *)TextFormat("%d", highScore));
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Texture2D grassTexture = LoadTexture("Assets/Textures/Gnd.jpg");
   
   
    Font  font =LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");

    GameState state = STATE_MENU;

   
        // ---- UPDATE ----
 


    while(!WindowShouldClose())
    {   

        float dt = GetFrameTime();
       

         switch (state) {
            case STATE_MENU:{
                break; // Handled in draw section via buttons
            }
            case STATE_PLAYING:{
              
       position.x = position.x+speed.x*dt;
        position.y = position.y+speed.y*dt;


                if (IsKeyPressed(KEY_ESCAPE)) {
                    if (score > highScore) { 
                        highScore = score; 
                        SaveHighScore(); 
                    }
                    state = STATE_GAMEOVER;
                }

        if(position.y-BALL_RADIUS < 0) //vertical wall
        {
            position.y = BALL_RADIUS;
            speed.y = -speed.y;
        }
        else if(position.y + BALL_RADIUS > HEIGHT)
        {
            position.y = HEIGHT - BALL_RADIUS;
            speed.y = -speed.y;
        }
        
        if(position.x - BALL_RADIUS <0)
        {
            position.x = BALL_RADIUS;
            speed.x = -speed.x;
        }
        else if(position.x+BALL_RADIUS > WIDTH)
        {
            position.x = WIDTH-BALL_RADIUS;
            speed.x = -speed.x;
        }
        // Update mouse position while button is held
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && Vector2Length(speed) <STOP_THRESHOLD && startedplaying == 0)
        {
            startedplaying = 1;
            shotcount++;
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && startedplaying == 1)
        {
            mouse = GetMousePosition();
               DrawLineV(position, mouse, BLACK);      
        }
        
      
        if(shotcount == 1 && startedplaying == 1)
        {
           score = 25;

        }
    else  if(shotcount > 1 && shotcount <=5&& startedplaying == 1)                                                                                                                 
        {
           score = 20 ;


        }
         else if(shotcount > 5&& shotcount <= 10&& startedplaying == 1)                                                                                                                 
        
        {
           score = 10 ;
        }
    
        else if(shotcount > 10&& startedplaying == 1)                                                                                                                 
        
        {
           score = 5 ;
        }

       



        Vector2 Diff_hole_ball = {(position.x-hole.x),(position.y-hole.y)} ;

      

        if (!hasWon && Vector2Length(Diff_hole_ball)>=0 && Vector2Length(Diff_hole_ball)<10.0f){
            hasWon = true;
            position.x = (WIDTH*1.0)/10.0f;
            position.y = HEIGHT/2.0f ;
             textDisplayTimer = 2.0f;
          
               
       
        }


        if(textDisplayTimer > 0.0f) { 
            DrawTextEx(font, "congratulations!\n       You Won!\n", (Vector2){100, 100}, 40, 2, WHITE);
            textDisplayTimer -= dt;
        }

        if(textDisplayTimer <= 0.0f && hasWon) {
            if (score > highScore) { 
                highScore = score; 
                SaveHighScore(); 
            }
         
            state = STATE_GAMEOVER;
            hasWon = false;

        }

        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && startedplaying == 1)
        {
            mouse = GetMousePosition();
            Vector2 shotdir = {position.x - mouse.x, position.y - mouse.y};
            
            // Calculate drag distance and cap it
            float dragDistance = Vector2Length(shotdir);
            if(dragDistance > MAX_SPEED) dragDistance = MAX_SPEED;  // Cap maximum speed
            
            // Normalize the direction and apply speed based on drag distance
            shotdir = Vector2Normalize(shotdir);
            speed.x = 3 * shotdir.x  * dragDistance;
            speed.y = 3 * shotdir.y * dragDistance;
            startedplaying = 0;
        }
        
       
        if(speed.x>0) speed.x -= DECELARATION * (fabsf(speed.x)/Vector2Length(speed)) * dt;
        if(speed.x<0) speed.x += DECELARATION * (fabsf(speed.x)/Vector2Length(speed)) * dt;
        if(speed.y>0) speed.y -= DECELARATION * (fabsf(speed.y)/Vector2Length(speed)) * dt;
        if(speed.y<0) speed.y += DECELARATION * (fabsf(speed.y)/Vector2Length(speed)) * dt;
        
        if(Vector2Length(speed)< STOP_THRESHOLD) 
        {
            speed.x = 0;
            speed.y = 0;
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

           switch (state) {
            case STATE_MENU: {
                  ClearBackground(RAYWHITE);

                DrawText("GOLF MANIA", 300, 100, 40, DARKBLUE);
                if (ButtonPressed((Rectangle){300, 220, 200, 50}, "Start", 20)) { 
                    score = 0; 
                    hasWon = false;
                    startedplaying = 0;
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

             case STATE_PLAYING: {
              ClearBackground(BROWN);
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
                  DrawText(TextFormat("Shot Count: %d", shotcount), 10, 40, 20, BLACK);
              

                DrawText("Press ESC to end", 10, 70, 20, GRAY);
           
      DrawTextureV(grassTexture, (Vector2){WIDTH/2 - grassTexture.width/2, HEIGHT/2 - grassTexture.height/2}, WHITE);
        
        DrawCircleV(hole,HOLE_RADIUS,BLACK);
        if(!hasWon)
        {
            DrawCircle(position.x, position.y, BALL_RADIUS+2.0f, BLACK);
            DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
        }
       
      break;
    }
            case STATE_HIGHSCORE: {
                ClearBackground(RAYWHITE);
                DrawText(TextFormat("High Score: %d", highScore), 280, 250, 30, MAROON);
                DrawText("Press BACKSPACE to go back", 220, 320, 20, GRAY);
                break;
            }
            case STATE_GAMEOVER:{
                ClearBackground(RAYWHITE);
                DrawText(TextFormat("Game Over! Score: %d", score), 200, 250, 30, RED);
                shotcount = 0;
                DrawText("Press ENTER for menu", 250, 320, 20, GRAY);
    
                break;
        }
    }
  
        
        EndDrawing();
    }
     UnloadFont(font);
     UnloadTexture(grassTexture);
    CloseWindow();
    return 0;
}  
