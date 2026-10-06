#include "raylib.h"
#include "raymath.h"

#define WIDTH 1000
#define HEIGHT 600 
#define BALL_SPEED 500
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 1000.0f  // Maximum speed based on drag distance
#define DECELARATION 100.0f
#define OBSTACLE_WIDTH 31.0f
#define OBSTACLE_HEIGHT 353.0f

Vector2 position = {(WIDTH*1.0)/10.0f, HEIGHT/2.0f}; //ini pos of ball
Vector2 speed = {0.0f, 0.0f}; //initially at rest
Vector2 collisionP;
Vector2 mouse;
Vector2 hole = {(WIDTH*9.0)/10.0f, HEIGHT/2.0f};

Rectangle obstacle_1 = {247.5f, 0.0f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle obstacle_2 = {477.5f, 247.5f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};
Rectangle obstacle_3 = {707.5f, 0.0f, OBSTACLE_WIDTH, OBSTACLE_HEIGHT};

// Bounces the ball off an axis-aligned rectangle.
// Works out the point of the rectangle nearest to the ball centre, uses the
// direction from that point to the centre as the surface normal, reflects the
// velocity about it, then lifts the ball back out to resting contact.
static void ResolveCollision(Rectangle obstacle)
{
    if (!CheckCollisionCircleRec(position, BALL_RADIUS, obstacle)) return;

    float left   = obstacle.x;
    float right  = obstacle.x + obstacle.width;
    float top    = obstacle.y;
    float bottom = obstacle.y + obstacle.height;

    Vector2 contact = { Clamp(position.x, left, right), Clamp(position.y, top, bottom) };
    Vector2 normal = Vector2Subtract(position, contact);

    // A fast shot can end a frame with the centre already inside the obstacle.
    // Then the nearest point is the centre itself and there is no normal to be
    // had, so push out through whichever face is closest.
    if (Vector2Length(normal) < 0.0001f)
    {
        float dLeft   = position.x - left;
        float dRight  = right - position.x;
        float dTop    = position.y - top;
        float dBottom = bottom - position.y;
        float least = fminf(fminf(dLeft, dRight), fminf(dTop, dBottom));

        if (least == dLeft)        { contact.x = left;   normal = (Vector2){ -1.0f, 0.0f }; }
        else if (least == dRight)  { contact.x = right;  normal = (Vector2){  1.0f, 0.0f }; }
        else if (least == dTop)    { contact.y = top;    normal = (Vector2){  0.0f, -1.0f }; }
        else                       { contact.y = bottom; normal = (Vector2){  0.0f,  1.0f }; }
    }

    normal = Vector2Normalize(normal);
    speed = Vector2Reflect(speed, normal);

    // The small extra gap matters: CheckCollisionCircleRec counts exact contact
    // as a collision, so resting the ball flush would re-trigger this next frame.
    position = Vector2Add(contact, Vector2Scale(normal, BALL_RADIUS + 0.01f));
}

static void ResolveObstacles(void)
{
    ResolveCollision(obstacle_1);
    ResolveCollision(obstacle_2);
    ResolveCollision(obstacle_3);
}

// Moves the ball by dt in hops no longer than half its radius, checking for a
// bounce after each hop. A full-power shot covers 50 px in one frame, which is
// wider than the 31 px obstacles: moved in one go the ball can finish the frame
// already past the face it should have hit, and then gets thrown out of the far
// side (or missed entirely). Small hops keep the ball on the side it came from.
static void MoveBall(float dt)
{
    int hops = (int)(Vector2Length(speed)*dt/(BALL_RADIUS*0.5f)) + 1;
    float h = dt/hops;

    for (int i = 0; i < hops; i++)
    {
        position.x = position.x + speed.x*h;
        position.y = position.y + speed.y*h;

        ResolveObstacles();
    }
}

float textDisplayTimer = 0.0f;

int main()
{
    InitWindow(WIDTH, HEIGHT, "Golf Mania");
    SetTargetFPS(60);

    Font  font =LoadFont("Assets/Fonts/Minecrafter-MA3Dw.ttf");

    while(!WindowShouldClose())
    {   
        float dt = GetFrameTime();

        MoveBall(dt);

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
        
        if(position.x - BALL_RADIUS <0)   //Horizontal Wall
        {
            position.x = BALL_RADIUS;
            speed.x = -speed.x;
        }
        else if(position.x+BALL_RADIUS > WIDTH)
        {
            position.x = WIDTH-BALL_RADIUS;
            speed.x = -speed.x;
        }

        // obstacle_2 runs from y=247.5 to y=600.5, so it pokes past the bottom
        // edge of the screen. The wall clamp above can therefore shove the ball
        // sideways back into it; check once more now the ball is inside the walls.
        ResolveObstacles();

        //obstacle conditions
        // if(position.x+BALL_RADIUS>297.5f && position.y<OBSTACLE_HEIGHT)
        // {
        //     position.x = 297.5-BALL_RADIUS;
        //     speed.x = -speed.x;
        // }
        // else if((position.x>297.5f && position.x<297.5f+OBSTACLE_WIDTH) && position.y-BALL_RADIUS<OBSTACLE_HEIGHT)
        // {
        //     position.y = OBSTACLE_HEIGHT+BALL_RADIUS;
        //     speed.y = -speed.y; 
        // }
        // else if(position.x-BALL_RADIUS < 297.5+OBSTACLE_WIDTH && position.y<OBSTACLE_HEIGHT)
        // {
        //     position.x = 297.5+OBSTACLE_HEIGHT+BALL_RADIUS;
        //     speed.x = -speed.x;
        // }

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
        ClearBackground(DARKGREEN);
        DrawCircleV(hole,HOLE_RADIUS,BLACK);
        DrawCircle(position.x, position.y, BALL_RADIUS+2.0f, BLACK);
        DrawCircle(position.x, position.y, BALL_RADIUS, WHITE);
        DrawRectangleRec(obstacle_1, BLACK); //border
        DrawRectangle(250.0f, 0.0f, OBSTACLE_WIDTH-6.0f, OBSTACLE_HEIGHT-3.0f, GRAY);
        DrawRectangleRec(obstacle_2, BLACK); //border
        DrawRectangle(480.0f, 250.0f, OBSTACLE_WIDTH-6.0f, OBSTACLE_HEIGHT-3.0f, GRAY);
        DrawRectangleRec(obstacle_3, BLACK);  //border
        DrawRectangle(710.0f, 0.0f, OBSTACLE_WIDTH-6.0f, OBSTACLE_HEIGHT-3.0f, GRAY);
        
       
        
        EndDrawing();
    }
     UnloadFont(font);
    CloseWindow();
    return 0;
}  
