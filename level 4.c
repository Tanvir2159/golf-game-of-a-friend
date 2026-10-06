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
#define POND_RADIUS 100.0f
#define GRAYZONE_DECELARATION 300.0f

Vector2 position = {(WIDTH*19.0f)/20.0f, (HEIGHT*11.0f)/12.0f}; //ini pos of ball
Vector2 initialPosition = {(WIDTH*19.0f)/20.0f, (HEIGHT*11.0f)/12.0f};
Vector2 speed = {0.0f, 0.0f}; //initially at rest
Vector2 mouse;
Vector2 hole = {WIDTH/2.0f, HEIGHT/2.0f};
Vector2 smallPond = {700.0f, 400.0f};

Rectangle largePond = {100.0f, 100.0f, 200.0f, 400.0f};
//Rectangle smallPond = {650.0f, 350.0f, 200.0f, 200.0f};

float textDisplayTimer = 0.0f;

static const char *circleTextureFragmentShader =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec2 offset = fragTexCoord - vec2(0.5);\n"
    "    if (dot(offset, offset) > 0.25) discard;\n"
    "    finalColor = texture(texture0, fragTexCoord) * fragColor;\n"
    "}\n";

int main()
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    InitAudioDevice();

    Texture2D grassTexture = LoadTexture("Assets/Textures/grass03.png");
    Texture2D waterTexture = LoadTexture("Assets/Textures/tex_Water.png");
    Shader circleTextureShader = LoadShaderFromMemory(0, circleTextureFragmentShader);
    
    Sound waterSplash = LoadSound("Assets/Audio/Water splash.ogg");
   
    Font  font = LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");


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

        Vector2 Diff_hole_ball = {(position.x-hole.x),(position.y-hole.y)} ;

        static bool hasWon = false;
        static bool hitPond = false;

        if (!hasWon && !hitPond &&
            (Vector2Distance(position, smallPond) < POND_RADIUS + BALL_RADIUS ||
             CheckCollisionCircleRec(position, BALL_RADIUS, largePond)))
        {
            hitPond = true;
            PlaySound(waterSplash);
            speed = (Vector2){0.0f, 0.0f};
            position = initialPosition;
            textDisplayTimer = 2.0f;
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
        ClearBackground(DARKGREEN);
      
        DrawTextureV(grassTexture, (Vector2){WIDTH/2 - grassTexture.width/2, HEIGHT/2 - grassTexture.height/2}, WHITE);
        
        
        if (!hasWon && !hitPond && Vector2Length(Diff_hole_ball) < 10.0f){
            hasWon = true;
            speed = (Vector2){0.0f, 0.0f};
            position = initialPosition;
            textDisplayTimer = 2.0f;
        }

        DrawRectangleRec(largePond, DARKBLUE);
        DrawCircleV(smallPond, POND_RADIUS, DARKBLUE);
        DrawTexturePro(
            waterTexture,
            (Rectangle){0.0f, 0.0f, (float)waterTexture.width, (float)waterTexture.height},
            largePond,
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE
        );
        BeginShaderMode(circleTextureShader);
        DrawTexturePro(
            waterTexture,
            (Rectangle){0.0f, 0.0f, (float)waterTexture.width, (float)waterTexture.height},
            (Rectangle){smallPond.x - POND_RADIUS, smallPond.y - POND_RADIUS,
                        POND_RADIUS * 2.0f, POND_RADIUS * 2.0f},
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE
        );
        EndShaderMode();

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            mouse = GetMousePosition();
            DrawLineV(position, mouse, BLACK);            
        }
        
        if(textDisplayTimer > 0.0f)
        {
            const char *message = hitPond ? "Oh no!\nYou hit the pond!" : "Congratulations!\nYou won!";
            DrawTextEx(font, message, (Vector2){100, 100}, 40, 2, WHITE);
            textDisplayTimer -= dt;
        }
        else
        {
            hasWon = false;
            hitPond = false;
        }

        DrawCircleV(hole,HOLE_RADIUS,BLACK);
        if(!hasWon && !hitPond)
        {
            DrawCircle(position.x, position.y, BALL_RADIUS+2.0f, BLACK);
            DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
        }
       
        
        EndDrawing();
    }
    UnloadFont(font);
    UnloadTexture(grassTexture);
    UnloadTexture(waterTexture);
    UnloadSound(waterSplash);
    UnloadShader(circleTextureShader);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}  
