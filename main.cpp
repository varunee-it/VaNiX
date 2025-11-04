#include <iostream>
#include <conio.h>  // For _kbhit() and _getch()
#include <windows.h> // For Sleep()
#include <deque>
#include <cstdlib>
#include <ctime>

using namespace std;

// Cell class to represent a position on the grid
class Cell {
private:
    int x, y;

public:
    Cell(int x = 0, int y = 0) : x(x), y(y) {}
    
    int getX() const { return x; }
    int getY() const { return y; }
    
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    
    bool equals(const Cell& other) const {
        return x == other.x && y == other.y;
    }
};

// Direction enum
enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Snake class using deque (double-ended queue)
class Snake {
private:
    deque<Cell> body;
    Direction currentDirection;
    Direction nextDirection;
    bool growing;

public:
    Snake(int startX, int startY) {
        // Initialize snake with 3 cells
        body.push_back(Cell(startX, startY));
        body.push_back(Cell(startX - 1, startY));
        body.push_back(Cell(startX - 2, startY));
        
        currentDirection = RIGHT;
        nextDirection = RIGHT;
        growing = false;
    }
    
    Cell getHead() const {
        return body.front();
    }
    
    const deque<Cell>& getBody() const {
        return body;
    }
    
    void setDirection(Direction dir) {
        // Prevent 180-degree turns
        if ((currentDirection == UP && dir == DOWN) ||
            (currentDirection == DOWN && dir == UP) ||
            (currentDirection == LEFT && dir == RIGHT) ||
            (currentDirection == RIGHT && dir == LEFT)) {
            return;
        }
        nextDirection = dir;
    }
    
    void move() {
        currentDirection = nextDirection;
        Cell head = getHead();
        Cell newHead = head;
        
        // Calculate new head position based on direction
        switch (currentDirection) {
            case UP:    newHead.setY(head.getY() - 1); break;
            case DOWN:  newHead.setY(head.getY() + 1); break;
            case LEFT:  newHead.setX(head.getX() - 1); break;
            case RIGHT: newHead.setX(head.getX() + 1); break;
        }
        
        // Add new head to front (enqueue)
        body.push_front(newHead);
        
        // Remove tail if not growing (dequeue)
        if (!growing) {
            body.pop_back();
        } else {
            growing = false;
        }
    }
    
    void grow() {
        growing = true;
    }
    
    bool checkSelfCollision() const {
        Cell head = getHead();
        for (size_t i = 1; i < body.size(); i++) {
            if (head.equals(body[i])) {
                return true;
            }
        }
        return false;
    }
    
    bool isOccupying(const Cell& cell) const {
        for (const auto& segment : body) {
            if (segment.equals(cell)) {
                return true;
            }
        }
        return false;
    }
};

// Food class
class Food {
private:
    Cell position;

public:
    Food() : position(0, 0) {}
    
    void spawn(const Snake& snake, int gridSize) {
        // Ensure food doesn't spawn on snake's body
        bool validPosition = false;
        int attempts = 0;
        
        while (!validPosition && attempts < 100) {
            int x = rand() % gridSize;
            int y = rand() % gridSize;
            position = Cell(x, y);
            
            if (!snake.isOccupying(position)) {
                validPosition = true;
            }
            attempts++;
        }
    }
    
    Cell getPosition() const {
        return position;
    }
};

// GameBoard class to manage the game
class GameBoard {
private:
    int gridSize;
    Snake* snake;
    Food* food;
    int score;
    int highScore;
    bool gameOver;
    bool paused;
    int speed;

public:
    GameBoard(int size) : gridSize(size), score(0), highScore(0), 
                          gameOver(false), paused(false), speed(100) {
        snake = new Snake(size / 2, size / 2);
        food = new Food();
        food->spawn(*snake, gridSize);
    }
    
    ~GameBoard() {
        delete snake;
        delete food;
    }
    
    void processInput() {
        if (_kbhit()) {
            char key = _getch();
            
            // Handle arrow keys (they send two characters)
            if (key == -32 || key == 0) {
                key = _getch();
                switch (key) {
                    case 72: snake->setDirection(UP); break;    // Up arrow
                    case 80: snake->setDirection(DOWN); break;  // Down arrow
                    case 75: snake->setDirection(LEFT); break;  // Left arrow
                    case 77: snake->setDirection(RIGHT); break; // Right arrow
                }
            } else {
                // WASD controls
                switch (key) {
                    case 'w': case 'W': snake->setDirection(UP); break;
                    case 's': case 'S': snake->setDirection(DOWN); break;
                    case 'a': case 'A': snake->setDirection(LEFT); break;
                    case 'd': case 'D': snake->setDirection(RIGHT); break;
                    case 'p': case 'P': paused = !paused; break;
                    case 27: gameOver = true; break; // ESC key
                }
            }
        }
    }
    
    void update() {
        if (paused || gameOver) return;
        
        snake->move();
        
        // Check collision with boundaries
        Cell head = snake->getHead();
        if (head.getX() < 0 || head.getX() >= gridSize ||
            head.getY() < 0 || head.getY() >= gridSize) {
            gameOver = true;
            return;
        }
        
        // Check self collision
        if (snake->checkSelfCollision()) {
            gameOver = true;
            return;
        }
        
        // Check if snake ate food
        if (head.equals(food->getPosition())) {
            snake->grow();
            score += 10;
            
            // Increase speed slightly
            if (speed > 50) {
                speed -= 2;
            }
            
            food->spawn(*snake, gridSize);
        }
    }
    
    void render() {
        system("cls"); // Clear screen
        
        // Display title and score
        cout << "======================================================\n";
        cout << "            SNAKE GAME - IT603 PROJECT               \n";
        cout << "======================================================\n";
        cout << "Score: " << score << "  |  High Score: " << highScore;
        if (paused) cout << "  |  [PAUSED]";
        cout << "\n\n";
        
        // Create grid (2D array representation)
        char grid[25][25];
        
        // Initialize grid with empty spaces
        for (int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                grid[i][j] = ' ';
            }
        }
        
        // Place food
        Cell foodPos = food->getPosition();
        grid[foodPos.getY()][foodPos.getX()] = '*';
        
        // Place snake
        const deque<Cell>& body = snake->getBody();
        for (size_t i = 0; i < body.size(); i++) {
            Cell segment = body[i];
            if (i == 0) {
                grid[segment.getY()][segment.getX()] = 'O'; // Head
            } else {
                grid[segment.getY()][segment.getX()] = 'o'; // Body
            }
        }
        
        // Draw grid with borders
        cout << "+";
        for (int i = 0; i < gridSize; i++) cout << "-";
        cout << "+\n";
        
        for (int i = 0; i < gridSize; i++) {
            cout << "|";
            for (int j = 0; j < gridSize; j++) {
                cout << grid[i][j];
            }
            cout << "|\n";
        }
        
        cout << "+";
        for (int i = 0; i < gridSize; i++) cout << "-";
        cout << "+\n";
        
        // Display controls
        cout << "\nControls: W/A/S/D or Arrow Keys to move\n";
        cout << "          P to Pause  |  ESC to Exit\n";
    }
    
    void displayGameOver() {
        system("cls");
        cout << "\n\n";
        cout << "======================================================\n";
        cout << "                    GAME OVER!                       \n";
        cout << "======================================================\n\n";
        cout << "        Final Score: " << score << "\n";
        cout << "        High Score:  " << highScore << "\n\n";
        cout << "        Press 'R' to Restart or 'Q' to Quit\n\n";
        
        if (score > highScore) {
            highScore = score;
        }
    }
    
    bool isGameOver() const {
        return gameOver;
    }
    
    bool isPaused() const {
        return paused;
    }
    
    int getSpeed() const {
        return speed;
    }
    
    void reset() {
        delete snake;
        delete food;
        
        snake = new Snake(gridSize / 2, gridSize / 2);
        food = new Food();
        food->spawn(*snake, gridSize);
        
        score = 0;
        gameOver = false;
        paused = false;
        speed = 100;
    }
    
    int getHighScore() const {
        return highScore;
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));
    
    const int GRID_SIZE = 20;
    GameBoard game(GRID_SIZE);
    
    cout << "======================================================\n";
    cout << "         Welcome to Snake Game - IT603!              \n";
    cout << "======================================================\n\n";
    cout << "Instructions:\n";
    cout << "- Use W/A/S/D or Arrow Keys to control the snake\n";
    cout << "- Eat food (*) to grow and increase score\n";
    cout << "- Avoid hitting walls or yourself\n";
    cout << "- Press P to pause, ESC to exit\n\n";
    cout << "Press any key to start...\n";
    _getch();
    
    // Main game loop
    while (true) {
        if (!game.isGameOver()) {
            game.processInput();
            game.update();
            game.render();
            Sleep(game.getSpeed());
        } else {
            game.displayGameOver();
            
            char choice = _getch();
            if (choice == 'r' || choice == 'R') {
                game.reset();
            } else if (choice == 'q' || choice == 'Q') {
                break;
            }
        }
    }
    
    cout << "\nThanks for playing! Final High Score: " << game.getHighScore() << "\n";
    return 0;
}