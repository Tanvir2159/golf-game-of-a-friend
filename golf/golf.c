#include "raylib.h"
#include "raymath.h"

#define WIDTH 800
#define HEIGHT 600 
#define BALL_SPEED 500
#define BALL_RADIUS 12.0f
#define STOP_THRESHOLD 5.0

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Texture2D grassTexture = LoadTexture("Assets/Textures/ground_grass_gen_10.png");
    Rectangle groundrect = {WIDTH/6, HEIGHT/6, (WIDTH/6)*4, (HEIGHT/6)*4};
    float ground = (HEIGHT*2.0)/3;
    
    typedef struct {
        Vector2 position;
        Vector2 velocity;
        float radius;
        Color color;
    } GolfBall;
    GolfBall ball = {{WIDTH/2.0f, HEIGHT/2.0f}, {0.0f, 0.0f}, 12.0f, RAYWHITE};
    
    bool dragging = false;
    Vector2 dragStart={0,0};
    Vector2 dragCurrent={0,0};

    float dt = GetFrameTime();
    
        
    while(!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();
        //if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        // {
        //     //dragging = true;
        //     //dragStart = mouse;
        //     //dragCurrent = mouse;

        // }
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            //dragCurrent = mouse;
            DrawLineV(ball.position, mouse, BLACK);

        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            Vector2 shotDir = Vector2Subtract(ball.position, mouse);
            float shotLength = Vector2Length(shotDir);

            if(shotLength > 0.0f)
            {
                //shotDir = Vector2Normalize(shotDir);
                ball.velocity = Vector2Scale(shotDir, BALL_SPEED);
            }

            //dragging = false;
        }

        ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, dt));
        ball.velocity = Vector2Scale(ball.velocity, 0.995f);

        if(Vector2Length(ball.velocity)<5.0f)
        {
            ball.velocity = (Vector2){0,0};
        }

        
        BeginDrawing();
        DrawTexturePro(
            grassTexture,
            (Rectangle){ 0, 0, (float)grassTexture.width, (float)grassTexture.height },
            (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
        DrawRectangleRec(groundrect, (Color){ 133, 133, 133, 150 });
        DrawCircleV(ball.position, ball.radius+1.0f, BLACK);
        DrawCircleV(ball.position, ball.radius, ball.color);

        // if(dragging)
        // {
        // }

        EndDrawing();

    }
    UnloadTexture(grassTexture);
    CloseWindow();
    return 0;
}