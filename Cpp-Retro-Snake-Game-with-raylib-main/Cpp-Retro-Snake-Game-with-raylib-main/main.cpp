#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

static bool allowMove = false;
Color background = {139, 69, 19, 255};  // Brown background
Color outsideBackground = {0, 0, 0, 255}; // Black background outside the boundary
Color snakeColor = {0, 255, 0, 255}; // Bright green snake (default)
Color borderColor = {255, 165, 0, 255}; // Orange border
Color foodColor = {255, 255, 0, 255};  // Bright yellow food
Color diamondColor = {0, 191, 255, 255}; // Light blue diamond food
Color hurdleColor = {255, 20, 147, 255}; // Deep pink hurdle color
Color gameOverColor = {255, 0, 0, 255}; // Red color for game over text
Color titleColor = {135, 206, 235, 255}; // Sky blue color for Snake Mania title

// Array of colors for the snake to cycle through (removed yellow and pink)
Color snakeColors[] = {
    {0, 255, 0, 255},    // Green
    {255, 0, 0, 255},    // Red
    {0, 0, 255, 255},    // Blue
    {0, 255, 255, 255},  // Cyan
    {255, 0, 255, 255},  // Magenta
    {255, 165, 0, 255},  // Orange
    {0, 128, 128, 255},  // Teal
    {128, 0, 128, 255}   // Purple
};
int currentColorIndex = 0;

int cellSize = 30;
int cellCount = 25;
int offset = 75;
double lastUpdateTime = 0;
double lastDiamondTime = 0; // Timer for diamond food
bool diamondFoodActive = false; // Flag for diamond food
Vector2 diamondFoodPosition; // Position of the diamond food
double diamondFoodSpawnTime = 0; // Time when diamond food spawned
const double diamondFoodDuration = 3.0; // Duration for diamond food (3 seconds)

double invincibilityStartTime = 0; // Timer for invincibility
bool invincible = false; // Flag for invincibility
const double invincibilityDuration = 20.0; // Duration of invincibility

int lastScoreMilestone = 0; // To track when we last changed the snake color

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            Rectangle segment = {offset + body[i].x * cellSize, offset + body[i].y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, snakeColors[currentColorIndex]);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (!addSegment)
        {
            body.pop_back();
        }
        addSegment = false;
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    Food(deque<Vector2> snakeBody)
    {
        position = GenerateRandomPos(snakeBody);
    }
    void Draw()
    {
        DrawCircle(offset + position.x * cellSize + cellSize / 2, offset + position.y * cellSize + cellSize / 2, cellSize / 3, foodColor);
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position;
        do
        {
            position = {GetRandomValue(0, cellCount - 1), GetRandomValue(0, cellCount - 1)};
        } while (ElementInDeque(position, snakeBody));
        return position;
    }
};

class Game
{
public:
    Snake snake;
    deque<Food> foodItems;
    bool running = true;
    int score = 0;
    int lives = 3; // New variable to track lives
    deque<Vector2> hurdles;

    Game()
    {
        for (int i = 0; i < 3; i++)
        {
            foodItems.push_back(Food(snake.body));
        }
        for (int i = 0; i < 10; i++)
        {
            Vector2 pos = {GetRandomValue(0, cellCount - 1), GetRandomValue(0, cellCount - 1)};
            hurdles.push_back(pos);
        }
    }

    void Draw()
    {
        // Draw the black background
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), outsideBackground);

        // Draw the brown area inside the boundary
        DrawRectangle(offset - 1, offset - 1, cellSize * cellCount + 2, cellSize * cellCount + 2, background);

        for (Food &food : foodItems)
        {
            food.Draw();
        }
        snake.Draw();
        DrawBorder();
        DrawHurdles();
        
        // Draw the game title at the center with a stylized font and new color
        DrawText("Snake Mania", GetScreenWidth() / 2 - MeasureText("Snake Mania", 40) / 2, 10, 40, titleColor);
        
        // Draw score at bottom right and lives at bottom left
        DrawScore(); // Call the method to draw the score
        DrawLives(); // Call the method to draw the lives

        // Draw diamond food if active
        if (diamondFoodActive)
        {
            DrawDiamondFood();
            
            // Show remaining time for diamond food
            float timeLeft = diamondFoodDuration - (GetTime() - diamondFoodSpawnTime);
            if (timeLeft > 0) {
                DrawText(TextFormat("%.1f", timeLeft), 
                         offset + diamondFoodPosition.x * cellSize - 5, 
                         offset + diamondFoodPosition.y * cellSize - 20, 
                         20, YELLOW);
            }
        }

        if (!running) // Display game over message
        {
            // Center the game over text and make it larger and red
            const char* gameOverText = "Game Over! Press R to Restart";
            int fontSize = 40; // Larger font size
            int textWidth = MeasureText(gameOverText, fontSize);
            int textX = GetScreenWidth() / 2 - textWidth / 2;
            int textY = GetScreenHeight() / 2 - fontSize / 2;
            
            // Draw a background box for better visibility
            DrawRectangle(textX - 20, textY - 20, textWidth + 40, fontSize + 40, BLACK);
            DrawRectangleLines(textX - 20, textY - 20, textWidth + 40, fontSize + 40, WHITE);
            
            // Draw the game over text
            DrawText(gameOverText, textX, textY, fontSize, gameOverColor);
        }
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithHurdles();
            CheckCollisionWithSelf(); // Check for self-collision
            CheckDiamondFoodTimer(); // Check if diamond food should spawn
            CheckDiamondFoodExpiry(); // Check if diamond food should expire
            CheckCollisionWithDiamondFood(); // Check for collision with diamond food
            CheckSnakeColorChange(); // Check if snake color should change
        }
    }

    void CheckCollisionWithSelf()
    {
        // Check if the snake's head collides with any part of its body (except the first segment)
        for (size_t i = 1; i < snake.body.size(); i++)
        {
            if (Vector2Equals(snake.body[0], snake.body[i]))
            {
                GameOver(); // Call GameOver if a collision is detected
                break;
            }
        }
    }

    void CheckSnakeColorChange()
    {
        // Check if score has reached a new multiple of 10
        int scoreMilestone = score / 10;
        if (scoreMilestone > lastScoreMilestone)
        {
            // Change to the next color in the array
            currentColorIndex = (currentColorIndex + 1) % (sizeof(snakeColors) / sizeof(snakeColors[0]));
            lastScoreMilestone = scoreMilestone;
        }
    }

    void CheckCollisionWithFood()
    {
        for (Food &food : foodItems)
        {
            if (Vector2Equals(snake.body[0], food.position))
            {
                food.position = food.GenerateRandomPos(snake.body);
                snake.addSegment = true;
                score++;
            }
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x < 0 || snake.body[0].x >= cellCount || snake.body[0].y < 0 || snake.body[0].y >= cellCount)
        {
            GameOver();
        }
    }

    void CheckCollisionWithHurdles()
    {
        if (ElementInDeque(snake.body[0], hurdles))
        {
            GameOver();
        }
    }

    void CheckDiamondFoodTimer()
    {
        if (!diamondFoodActive && GetTime() - lastDiamondTime >= 10.0) // Check if 10 seconds have passed since last diamond
        {
            lastDiamondTime = GetTime();
            diamondFoodPosition = {GetRandomValue(0, cellCount - 1), GetRandomValue(0, cellCount - 1)};
            // Ensure the diamond food does not spawn on the snake or hurdles
            while (ElementInDeque(diamondFoodPosition, snake.body) || ElementInDeque(diamondFoodPosition, hurdles))
            {
                diamondFoodPosition = {GetRandomValue(0, cellCount - 1), GetRandomValue(0, cellCount - 1)};
            }
            diamondFoodActive = true; // Activate diamond food
            diamondFoodSpawnTime = GetTime(); // Record when diamond food spawned
        }
    }

    void CheckDiamondFoodExpiry()
    {
        // Check if diamond food has been active for more than 3 seconds
        if (diamondFoodActive && GetTime() - diamondFoodSpawnTime >= diamondFoodDuration)
        {
            diamondFoodActive = false; // Deactivate diamond food after 3 seconds
        }
    }

    void CheckCollisionWithDiamondFood()
    {
        if (diamondFoodActive && Vector2Equals(snake.body[0], diamondFoodPosition))
        {
            score += 4; // Increase score by 4
            diamondFoodActive = false; // Deactivate diamond food
        }
    }

    void DrawDiamondFood()
    {
        Vector2 points[4] = {
            {offset + diamondFoodPosition.x * cellSize + cellSize / 2, offset + diamondFoodPosition.y * cellSize},
            {offset + diamondFoodPosition.x * cellSize, offset + diamondFoodPosition.y * cellSize + cellSize / 2},
            {offset + diamondFoodPosition.x * cellSize + cellSize / 2, offset + diamondFoodPosition.y * cellSize + cellSize},
            {offset + diamondFoodPosition.x * cellSize + cellSize, offset + diamondFoodPosition.y * cellSize + cellSize / 2}
        };
        DrawTriangle(points[0], points[1], points[2], diamondColor);
        DrawTriangle(points[0], points[2], points[3], diamondColor);
    }

    void GameOver()
    {
        lives--; // Decrease lives
        if (lives <= 0) // Check if lives are zero
        {
            running = false; // Stop the game
        }
        else
        {
            // Reset snake position and food if lives are still available
            snake.Reset();
            for (Food &food : foodItems)
            {
                food.position = food.GenerateRandomPos(snake.body);
            }
            diamondFoodActive = false; // Reset diamond food
        }
    }

    void Restart()
    {
        lives = 3; // Reset lives
        score = 0; // Reset score
        running = true; // Restart the game
        snake.Reset(); // Reset snake position
        for (Food &food : foodItems)
        {
            food.position = food.GenerateRandomPos(snake.body); // Reset food positions
        }
        diamondFoodActive = false; // Reset diamond food
        lastDiamondTime = 0; // Reset diamond food timer
        invincible = false; // Reset invincibility
        
        // Reset color tracking variables
        currentColorIndex = 0; // Reset to first color
        lastScoreMilestone = 0; // Reset milestone tracking
    }

    void DrawBorder()
    {
        int borderThickness = 5;
        DrawRectangleLinesEx({offset - borderThickness, offset - borderThickness, cellSize * cellCount + 2 * borderThickness, cellSize * cellCount + 2 * borderThickness}, borderThickness, borderColor);
    }

    void DrawHurdles()
    {
        for (size_t i = 0; i < hurdles.size(); i++)
        {
            DrawRectangle(offset + hurdles[i].x * cellSize, offset + hurdles[i].y * cellSize, cellSize, cellSize, hurdleColor); // Use the fixed pink color for hurdles
        }
    }

    void DrawScore() // New method to draw the score
    {
        DrawText(TextFormat("Score: %d", score), offset + 10, GetScreenHeight() - 40, 20, WHITE);
    }

    void DrawLives() // New method to draw the lives
    {
        DrawText(TextFormat("Lives: %d", lives), GetScreenWidth() - MeasureText(TextFormat("Lives: %d", lives), 20) - 10, GetScreenHeight() - 40, 20, WHITE); // Align lives to the bottom right
    }

    void UpdateInvincibility()
    {
        if (invincible && GetTime() - invincibilityStartTime >= invincibilityDuration)
        {
            invincible = false; // Deactivate invincibility after duration
        }
    }
};

int main()
{
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake Mania");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        if (EventTriggered(0.2))
        {
            allowMove = true;
            game.Update();
            game.UpdateInvincibility(); // Update invincibility status
        }

        // Only allow movement if the game is running
        if (game.running)
        {
            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove)
            {
                game.snake.direction = {0, -1};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
            {
                game.snake.direction = {0, 1};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove)
            {
                game.snake.direction = {-1, 0};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove)
            {
                game.snake.direction = {1, 0};
                allowMove = false;
            }
        }

        // Restart the game if the player presses 'R' and the game is over
        if (!game.running && IsKeyPressed(KEY_R))
        {
            game.Restart();
        }

        ClearBackground(background);
        game.Draw();
        EndDrawing();
    }

    CloseWindow(); 
    return 0;
}