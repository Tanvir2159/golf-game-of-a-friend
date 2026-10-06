#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define WIDTH 1000
#define HEIGHT 600 
#define BALL_SPEED 500
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 1000.0f  // Maximum speed based on drag distance
#define DECELARATION 100.0f

Vector2 position = {(WIDTH*1.0)/10.0f, HEIGHT/2.0f}; //ini pos of ball
Vector2 speed = {0.0f, 0.0f}; //initially at rest
Vector2 mouse;
Vector2 hole = {(WIDTH*9.0)/10.0f, HEIGHT/2.0f};

float textDisplayTimer = 0.0f;

int main()
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Texture2D grassTexture = LoadTexture("Assets/Textures/Gnd.jpg");
   
   
    Font  font =LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");


    while(!WindowShouldClose())
    {   

        float dt = GetFrameTime();
        position.x = position.x+speed.x*dt;
        position.y = position.y+speed.y*dt;

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
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            mouse = GetMousePosition();
            DrawLineV(position, mouse, BLACK);            
        }
        
        Vector2 Diff_hole_ball = {(position.x-hole.x),(position.y-hole.y)} ;

        static bool hasWon = false;

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


        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
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
        

        BeginDrawing();
        ClearBackground(BROWN);
      
      DrawTextureV(grassTexture, (Vector2){WIDTH/2 - grassTexture.width/2, HEIGHT/2 - grassTexture.height/2}, WHITE);
        
        DrawCircleV(hole,HOLE_RADIUS,BLACK);
        if(!hasWon)
        {
            DrawCircle(position.x, position.y, BALL_RADIUS+2.0f, BLACK);
            DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
        }
       
        
        EndDrawing();
    }
     UnloadFont(font);
     UnloadTexture(grassTexture);
    CloseWindow();
    return 0;
}  
