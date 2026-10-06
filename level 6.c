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
#define PIPE_WIDTH 550.0f

Vector2 position = {WIDTH/10.0f, HEIGHT/2.0f}; //ini pos of ball
Vector2 speed = {0.0f, 0.0f}; //initially at rest
Vector2 mouse;
Vector2 hole = {(WIDTH*9)/10.0f, HEIGHT/3.0f};

static float leftPipeDelayTimer = 0.0f;
static float rightPipeDelayTimer = 0.0f;

float textDisplayTimer = 0.0f;

// --- Pipe / spike hazard geometry (matches the rectangles used when drawing the textures) ---
static Rectangle leftSpikeRects[4] = {
    {200.0f, 0.0f,   100.0f, 130.0f},
    {200.0f, 130.0f, 100.0f, 130.0f},
    {200.0f, 325.0f, 100.0f, 140.0f},
    {200.0f, 465.0f, 100.0f, 140.0f}
};

static Rectangle rightSpikeRects[4] = {
    {650.0f, 0.0f,   100.0f, 140.0f},
    {650.0f, 140.0f, 100.0f, 145.0f},
    {650.0f, 345.0f, 100.0f, 130.0f},
    {650.0f, 470.0f, 100.0f, 130.0f}
};

// The gap between the spike segments on each side is the safe "mouth" of the pipe
static Rectangle leftMouth  = {200.0f, 260.0f, 100.0f, 65.0f};  // gap between segment 2 and 3 (y 260-325)
static Rectangle rightMouth = {650.0f, 285.0f, 100.0f, 60.0f};  // gap between segment 2 and 3 (y 285-345)

static bool gameOver = false;
static bool canTeleport = true;
static bool inPipe = false;          // true while the ball is "inside" the pipe waiting to emerge
static bool exitRight = false;       // true = will emerge from the right mouth, false = left mouth
static Vector2 pipeSpeed = {0.0f, 0.0f}; // speed carried through the pipe, applied on exit
#define PIPE_TRANSIT_TIME 2.0f

bool CircleRectOverlap(Vector2 center, float radius, Rectangle rect)
{
    float closestX = Clamp(center.x, rect.x, rect.x + rect.width);
    float closestY = Clamp(center.y, rect.y, rect.y + rect.height);
    float dx = center.x - closestX;
    float dy = center.y - closestY;
    return (dx * dx + dy * dy) < (radius * radius);
}


int main()
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Texture2D dirt = LoadTexture("Assets/Textures/Background L6.png");
    Texture2D pipe = LoadTexture("Assets/Textures/Pipe L5.png");
    Texture2D spike_left = LoadTexture("Assets/Textures/spikes left.png");
    Texture2D spike_right = LoadTexture("Assets/Textures/spikes right.png");
   
    Font  font = LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");

    while(!WindowShouldClose())
    {   

        float dt = GetFrameTime();
        static bool hasWon = false;
        Vector2 Diff_hole_ball = {(position.x-hole.x),(position.y-hole.y)};

        if(gameOver)
        {
            if(IsKeyPressed(KEY_R))
            {
                position = (Vector2){WIDTH/10.0f, HEIGHT/2.0f};
                speed = (Vector2){0.0f, 0.0f};
                gameOver = false;
                canTeleport = true;
            }
        }
        else
        {
        if(inPipe)
        {
            // Ball is "inside" the pipe (hidden) waiting to emerge on the other side
            leftPipeDelayTimer -= dt;
            if(leftPipeDelayTimer <= 0.0f)
            {
                if(exitRight)
                {
                    position.x = rightMouth.x + rightMouth.width - BALL_RADIUS - 1.0f;
                    position.y = Clamp(position.y, rightMouth.y + BALL_RADIUS, rightMouth.y + rightMouth.height - BALL_RADIUS);
                }
                else
                {
                    position.x = leftMouth.x + BALL_RADIUS + 1.0f;
                    position.y = Clamp(position.y, leftMouth.y + BALL_RADIUS, leftMouth.y + leftMouth.height - BALL_RADIUS);
                }

                speed = pipeSpeed;
                speed.y = 0.0f; // vertical component removed on exit
                if(speed.x > 0) speed.x -= DECELARATION * dt; // horizontal component still decelerates
                if(speed.x < 0) speed.x += DECELARATION * dt;

                inPipe = false;
                canTeleport = false; // don't immediately re-trigger while still sitting in the exit mouth
            }
        }
        else
        {
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

        

        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            mouse = GetMousePosition();
            Vector2 shotdir = {position.x - mouse.x, position.y - mouse.y};
            
            float dragDistance = Vector2Length(shotdir);
            if(3*dragDistance > MAX_SPEED) dragDistance = MAX_SPEED/3;  
            
            shotdir = Vector2Normalize(shotdir);
            speed.x = 9 * shotdir.x  * dragDistance;
            speed.y = 9 * shotdir.y * dragDistance;
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

        // --- Spike hazards: touching any spike segment ends the game ---
        for(int i = 0; i < 4; i++)
        {
            if(CircleRectOverlap(position, BALL_RADIUS, leftSpikeRects[i]) ||
               CircleRectOverlap(position, BALL_RADIUS, rightSpikeRects[i]))
            {
                gameOver = true;
                speed = (Vector2){0.0f, 0.0f};
            }
        }

        // --- Pipe mouths: entering one mouth starts a delay, then exits from the other ---
        if(!gameOver)
        {
            bool inLeftMouth  = CircleRectOverlap(position, BALL_RADIUS, leftMouth);
            bool inRightMouth = CircleRectOverlap(position, BALL_RADIUS, rightMouth);

            if(canTeleport && inLeftMouth)
            {
                inPipe = true;
                pipeSpeed = speed;
                speed = (Vector2){0.0f, 0.0f};
                leftPipeDelayTimer = PIPE_TRANSIT_TIME;
                exitRight = true; // entered from the left, so emerge from the right
                canTeleport = false;
            }
            else if(canTeleport && inRightMouth)
            {
                inPipe = true;
                pipeSpeed = speed;
                speed = (Vector2){0.0f, 0.0f};
                leftPipeDelayTimer = PIPE_TRANSIT_TIME;
                exitRight = false; // entered from the right, so emerge from the left
                canTeleport = false;
            }

            if(!inLeftMouth && !inRightMouth) canTeleport = true;
        }
        } // end of normal-movement (!inPipe) block

        } // end of !gameOver physics block

        BeginDrawing();
        ClearBackground(DARKGREEN);
        DrawTexturePro(dirt,
            (Rectangle){0.0f, 0.0f, (float)dirt.width, (float)dirt.height},
            (Rectangle){0.0f, 0.0f, (float)WIDTH, (float)HEIGHT},
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE);

        DrawTexturePro(pipe,
            (Rectangle){0.0f, 0.0f, (float)pipe.width, (float)pipe.height},
            (Rectangle){200.0f, 200.0f, 550.0f, 200.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE);

        DrawTexturePro(spike_left,
            (Rectangle){0.0f, 0.0f, (float)spike_left.width, (float)spike_left.height},
            (Rectangle){200.0f, 0.0f, 100.0f, 130.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);
        
        DrawTexturePro(spike_left,
            (Rectangle){0.0f, 0.0f, (float)spike_left.width, (float)spike_left.height},
            (Rectangle){200.0f, 130.0f, 100.0f, 130.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);    

        DrawTexturePro(spike_left,
            (Rectangle){0.0f, 0.0f, (float)spike_left.width, (float)spike_left.height},
            (Rectangle){200.0f, 325.0f, 100.0f, 140.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);
        DrawTexturePro(spike_left,
            (Rectangle){0.0f, 0.0f, (float)spike_left.width, (float)spike_left.height},
            (Rectangle){200.0f, 465.0f, 100.0f, 140.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);
        
        DrawTexturePro(spike_right,
            (Rectangle){0.0f, 0.0f, (float)spike_right.width, (float)spike_right.height},
            (Rectangle){650.0f, 0.0f, 100.0f, 140.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);    
        DrawTexturePro(spike_right,
            (Rectangle){0.0f, 0.0f, (float)spike_right.width, (float)spike_right.height},
            (Rectangle){650.0f, 140.0f, 100.0f, 145.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);
        DrawTexturePro(spike_right,
            (Rectangle){0.0f, 0.0f, (float)spike_right.width, (float)spike_right.height},
            (Rectangle){650.0f, 345.0f, 100.0f, 130.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);
        DrawTexturePro(spike_right,
            (Rectangle){0.0f, 0.0f, (float)spike_right.width, (float)spike_right.height},
            (Rectangle){650.0f, 470.0f, 100.0f, 130.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            LIGHTGRAY);        

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            mouse = GetMousePosition();
            DrawLineV(position, mouse, BLACK);            
        }
        
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
        else hasWon = false;

        DrawCircleV(hole,HOLE_RADIUS,BLACK);
        if(!hasWon && !inPipe)
        {
            DrawCircle(position.x, position.y, BALL_RADIUS+2.0f, BLACK);
            DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
        }

        if(gameOver)
        {
            DrawTextEx(font, "Game Over\nPress R to restart", (Vector2){WIDTH/2.0f - 220.0f, HEIGHT/2.0f - 40.0f}, 40, 2, RED);
        }

        EndDrawing();
    }
    UnloadFont(font);
    UnloadTexture(dirt);
    UnloadTexture(pipe);
    UnloadTexture(spike_left);
    UnloadTexture(spike_right);
    CloseWindow();
    return 0;
}  
