#include <gl2d/gl2d.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cmath>
#include <string>

// Window size
const int WIDTH = 640;
const int HEIGHT = 640;

// Structure to hold 2D points
struct Point {
    float x, y;
    Point(float _x, float _y) : x(_x), y(_y) {}
};

// Structure to hold color
struct Color {
    float r, g, b, a;
    Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
};

// Enemy types
enum class EnemyType {
    SKELETON,   // Blue, fast enemy
    ZOMBIE      // Red, normal enemy
};

// Function to get enemy name
std::string getEnemyName(EnemyType type) {
    switch (type) {
        case EnemyType::SKELETON:
            return "Skeleton";
        case EnemyType::ZOMBIE:
        default:
            return "Zombie";
    }
}

// Path waypoints
std::vector<Point> waypoints = {
    Point(352, 0), //starting point
    Point(352, 96),
    Point(160, 96),
    Point(160, 160),
    Point(96, 160),
    Point(96, 352),
    Point(224, 352),
    Point(224, 288),
    Point(416, 288),
    Point(416, 224),
    Point(544, 224),
    Point(544, 480),
    Point(288, 480),
    Point(288, 544),
    Point(-69, 544) //end point
};

// Structure for enemies
struct Enemy {
    Point pos;
    int currentWaypoint;
    bool isActive;
    EnemyType type;
    std::string name;
    
    Enemy() : pos(waypoints[0]), currentWaypoint(0), isActive(false), type(EnemyType::ZOMBIE) {
        name = getEnemyName(type);
    }
};

// Game settings
const float SPAWN_INTERVAL = 0.7f;    // Seconds between enemy spawns
const int MAX_ENEMIES = 20;            // Maximum number of enemies at once
const float ENEMY_SIZE = 32.0f;       // Size of enemy squares
const float ZOMBIE_SPEED = 100.0f;    // Zombie speed (slow)
const float SKELETON_SPEED = 200.0f;  // Skeleton speed (fast)

// Function to get enemy color based on type
Color getEnemyColor(EnemyType type) {
    switch (type) {
        case EnemyType::SKELETON:
            return Color(0.0f, 0.0f, 1.0f, 1.0f); // Blue for Skeleton
        case EnemyType::ZOMBIE:
        default:
            return Color(1.0f, 0.0f, 0.0f, 1.0f); // Red for Zombie
    }
}

// Function to get enemy speed based on type
float getEnemySpeed(EnemyType type) {
    switch (type) {
        case EnemyType::SKELETON:
            return SKELETON_SPEED;
        case EnemyType::ZOMBIE:
        default:
            return ZOMBIE_SPEED;
    }
}

// Function to process input
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

// Function to calculate distance between two points
float distance(const Point& a, const Point& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Function to update enemy position
void updateEnemy(Enemy& enemy, float deltaTime) {
    if (!enemy.isActive || enemy.currentWaypoint >= waypoints.size() - 1) return;

    Point& target = waypoints[enemy.currentWaypoint + 1];
    float dx = target.x - enemy.pos.x;
    float dy = target.y - enemy.pos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    
    float speed = getEnemySpeed(enemy.type);
    
    if (dist > 1.0f) {
        float moveX = (dx / dist) * speed * deltaTime;
        float moveY = (dy / dist) * speed * deltaTime;
        enemy.pos.x += moveX;
        enemy.pos.y += moveY;
    } else {
        enemy.currentWaypoint++;
        if (enemy.currentWaypoint >= waypoints.size() - 1) {
            enemy.isActive = false;
        }
    }
}

int main()
{
    // Print current working directory
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    
    // Print absolute path of the file we're trying to load
    std::filesystem::path filePath = "resources/background.png";
    std::cout << "Trying to load file from: " << std::filesystem::absolute(filePath) << std::endl;
    
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    // Create a window
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Tower Defense Game", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLAD (loads OpenGL functions)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    // Initialize GL2D
    gl2d::init();

    // Create GL2D renderer
    gl2d::Renderer2D renderer;
    renderer.create();

    // Load background texture
    gl2d::Texture backgroundTexture;
    std::cout << "Loading texture from: resources/background.png" << std::endl;
    
    // Check if file exists
    if (!std::filesystem::exists("resources/background.png")) {
        std::cerr << "ERROR: background.png not found in resources folder!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }
    
    backgroundTexture.loadFromFile("resources/background.png");

    if (backgroundTexture.id == 0)
    {
        std::cerr << "ERROR: Failed to load background texture!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    std::cout << "Successfully loaded background texture!" << std::endl;

    // Initialize enemies
    std::vector<Enemy> enemies(MAX_ENEMIES);
    float spawnTimer = 0.0f;

    // Main game loop
    float lastTime = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Update spawn timer
        spawnTimer += deltaTime;
        if (spawnTimer >= SPAWN_INTERVAL) {
            spawnTimer = 0.0f;
            // Find inactive enemy to spawn
            static int spawnCount = 0;  // Keep track of how many enemies we've spawned
            for (auto& enemy : enemies) {
                if (!enemy.isActive) {
                    enemy.isActive = true;
                    enemy.currentWaypoint = 0;
                    enemy.pos = waypoints[0];
                    // Alternate between Skeleton and Zombie
                    enemy.type = (spawnCount % 2 == 0) ? EnemyType::SKELETON : EnemyType::ZOMBIE;
                    enemy.name = getEnemyName(enemy.type);
                    std::cout << "Spawned " << enemy.name << std::endl;  // Print when enemy spawns
                    spawnCount++;
                    break;
                }
            }
        }

        // Update window metrics
        int w = 0, h = 0;
        glfwGetWindowSize(window, &w, &h);
        renderer.updateWindowMetrics(w, h);

        // Handle input
        processInput(window);

        // Update all active enemies
        for (auto& enemy : enemies) {
            updateEnemy(enemy, deltaTime);
        }

        // Clear screen
        renderer.clearScreen({0.1, 0.2, 0.6, 1});

        // Draw background
        renderer.renderRectangle({0, 0, WIDTH, HEIGHT}, backgroundTexture, {1, 1, 1, 1});

        // Draw all active enemies
        for (const auto& enemy : enemies) {
            if (enemy.isActive) {
                Color enemyColor = getEnemyColor(enemy.type);
                renderer.renderRectangle({enemy.pos.x - ENEMY_SIZE/2, enemy.pos.y - ENEMY_SIZE/2, 
                                        ENEMY_SIZE, ENEMY_SIZE}, 
                                        {enemyColor.r, enemyColor.g, enemyColor.b, enemyColor.a});
            }
        }

        // Flush renderer (draw everything)
        renderer.flush();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
