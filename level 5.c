 #include "raylib.h"
#include "raymath.h"
#include <math.h>

#define WIDTH 1000
#define HEIGHT 600
#define BALL_RADIUS 12.0f
#define HOLE_RADIUS 15.0f
#define STOP_THRESHOLD 15.0f
#define MAX_SPEED 1000.0f
#define DECELERATION 100.0f
#define SHOT_ACCELERATION 9.0f
#define MOVING_OBSTACLE_WIDTH 45.0f
#define MOVING_OBSTACLE_HEIGHT 190.0f
#define MOVING_OBSTACLE_SPEED 170.0f
#define OBSTACLE_X_GAP 500.0f

static Vector2 ballPosition = {80.0f, HEIGHT / 2.0f};
static Vector2 ballSpeed = {0.0f, 0.0f};
static const Vector2 ballStartPosition = {80.0f, HEIGHT / 2.0f};
static const Vector2 holePosition = {900.0f, HEIGHT / 2.0f};
static Rectangle movingObstacles[2] = {
	{225.0f, 80.0f, MOVING_OBSTACLE_WIDTH, MOVING_OBSTACLE_HEIGHT},
	{225.0f + OBSTACLE_X_GAP, 340.0f, MOVING_OBSTACLE_WIDTH, MOVING_OBSTACLE_HEIGHT}
};
static float obstacleDirections[2] = {1.0f, -1.0f};
static float obstacleHitCooldown = 0.0f;
static float messageTimer = 0.0f;
static int shotCount = 0;
static bool levelWon = false;
static bool exitRequested = false;

static void ResetLevel(void)
{
	ballPosition = ballStartPosition;
	ballSpeed = (Vector2){0.0f, 0.0f};
	movingObstacles[0].y = 80.0f;
	movingObstacles[1].y = 340.0f;
	obstacleDirections[0] = 1.0f;
	obstacleDirections[1] = -1.0f;
	obstacleHitCooldown = 0.0f;
	messageTimer = 0.0f;
	shotCount = 0;
	levelWon = false;
	exitRequested = false;
}

static void ResolveMovingObstacle(Rectangle obstacle, Sound obstacleHitSound)
{
	if (!CheckCollisionCircleRec(ballPosition, BALL_RADIUS, obstacle)) return;

	if (obstacleHitCooldown <= 0.0f && Vector2Length(ballSpeed) > STOP_THRESHOLD)
	{
		PlaySound(obstacleHitSound);
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

	normal = Vector2Normalize(normal);
	ballSpeed = Vector2Reflect(ballSpeed, normal);
	ballPosition = Vector2Add(contact, Vector2Scale(normal, BALL_RADIUS + 0.01f));
}

static void MoveBall(float dt, Sound obstacleHitSound)
{
	int steps = (int)(Vector2Length(ballSpeed) * dt / (BALL_RADIUS * 0.5f)) + 1;
	float stepTime = dt / steps;

	for (int step = 0; step < steps; step++)
	{
		ballPosition = Vector2Add(ballPosition, Vector2Scale(ballSpeed, stepTime));
		ResolveMovingObstacle(movingObstacles[0], obstacleHitSound);
		ResolveMovingObstacle(movingObstacles[1], obstacleHitSound);
	}
}

static void ApplyFriction(float dt)
{
	float speedLength = Vector2Length(ballSpeed);
	if (speedLength <= 0.0f) return;

	float reducedSpeed = fmaxf(0.0f, speedLength - DECELERATION * dt);
	ballSpeed = speedLength > 0.0f
		? Vector2Scale(ballSpeed, reducedSpeed / speedLength)
		: (Vector2){0.0f, 0.0f};

	if (reducedSpeed < STOP_THRESHOLD) ballSpeed = (Vector2){0.0f, 0.0f};
}

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Golf Mania - Level 5");
	InitAudioDevice();
	SetTargetFPS(60);

	Texture2D background = LoadTexture("Assets/Textures/Level background.png");
	Texture2D obstacleTexture = LoadTexture("Assets/Textures/pillar L2.png");
	Sound ballHitSound = LoadSound("Assets/Audio/Ball Hit.mp3");
	Sound obstacleHitSound = LoadSound("Assets/Audio/ball hit obs.wav");
	Sound levelCompletionSound = LoadSound("Assets/Audio/Level Completion.mp3");
	Sound wallHitSound = LoadSound("Assets/Audio/ball hit wall.mp3");

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		obstacleHitCooldown = fmaxf(0.0f, obstacleHitCooldown - dt);

		if (IsKeyPressed(KEY_R)) ResetLevel();

		if (!levelWon)
		{
			for (int obstacleIndex = 0; obstacleIndex < 2; obstacleIndex++)
			{
				Rectangle *obstacle = &movingObstacles[obstacleIndex];
				obstacle->y += obstacleDirections[obstacleIndex] * MOVING_OBSTACLE_SPEED * dt;
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

			MoveBall(dt, obstacleHitSound);

			if (ballPosition.y - BALL_RADIUS < 0.0f || ballPosition.y + BALL_RADIUS > HEIGHT)
			{
				ballPosition.y = Clamp(ballPosition.y, BALL_RADIUS, HEIGHT - BALL_RADIUS);
				ballSpeed.y = -ballSpeed.y;
				PlaySound(wallHitSound);
			}
			if (ballPosition.x - BALL_RADIUS < 0.0f || ballPosition.x + BALL_RADIUS > WIDTH)
			{
				ballPosition.x = Clamp(ballPosition.x, BALL_RADIUS, WIDTH - BALL_RADIUS);
				ballSpeed.x = -ballSpeed.x;
				PlaySound(wallHitSound);
			}

			ResolveMovingObstacle(movingObstacles[0], obstacleHitSound);
			ResolveMovingObstacle(movingObstacles[1], obstacleHitSound);

			if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && Vector2Length(ballSpeed) < STOP_THRESHOLD)
			{
				Vector2 drag = Vector2Subtract(ballPosition, GetMousePosition());
				float dragDistance = Vector2Length(drag);
				if (dragDistance > 0.0f)
				{
					ballSpeed = Vector2Scale(Vector2Normalize(drag), SHOT_ACCELERATION * dragDistance);
					shotCount++;
					PlaySound(ballHitSound);
				}
			}

			ApplyFriction(dt);

			if (Vector2Distance(ballPosition, holePosition) < HOLE_RADIUS)
			{
				PlaySound(levelCompletionSound);
				exitRequested = true;
			}
		}
		else if (messageTimer > 0.0f)
		{
			messageTimer -= dt;
		}
		else if (IsKeyPressed(KEY_ENTER))
		{
			ResetLevel();
		}

		if (exitRequested) break;

		BeginDrawing();
		DrawTexturePro(background,
					   (Rectangle){0, 0, (float)background.width, (float)background.height},
					   (Rectangle){0, 0, WIDTH, HEIGHT}, (Vector2){0, 0}, 0.0f, WHITE);
		DrawCircleV(holePosition, HOLE_RADIUS, BLACK);
		for (int obstacleIndex = 0; obstacleIndex < 2; obstacleIndex++)
		{
			DrawTexturePro(obstacleTexture,
						   (Rectangle){0, 0, (float)obstacleTexture.width, (float)obstacleTexture.height},
						   movingObstacles[obstacleIndex], (Vector2){0, 0}, 0.0f, WHITE);
		}

		if (!levelWon)
		{
			DrawCircleV(ballPosition, BALL_RADIUS + 2.0f, BLACK);
			DrawCircleV(ballPosition, BALL_RADIUS, WHITE);
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !levelWon)
		{
			Vector2 shotDirection = Vector2Subtract(ballPosition, GetMousePosition());
			float dragDistance = fminf(Vector2Length(shotDirection), MAX_SPEED);

			if (dragDistance > 0.0f)
			{
				shotDirection = Vector2Normalize(shotDirection);
				const int dotCount = 8;
				const float dotRadius = 4.0f;
				float spacing = (dragDistance * 3.0f) / dotCount;

				for (int dotIndex = 1; dotIndex <= dotCount; dotIndex++)
				{
					Vector2 dotPosition = Vector2Add(
						ballPosition,
						Vector2Scale(shotDirection, spacing * dotIndex));
					DrawCircleV(dotPosition, dotRadius, BLACK);
				}
			}
		}
		EndDrawing();
	}

	UnloadTexture(background);
	UnloadTexture(obstacleTexture);
	UnloadSound(ballHitSound);
	UnloadSound(obstacleHitSound);
	UnloadSound(levelCompletionSound);
	UnloadSound(wallHitSound);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
