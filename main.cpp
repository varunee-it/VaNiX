#include <iostream>
#include <conio.h>
#include <windows.h>
#include <deque>
#include <cstdlib>
#include <ctime>
using namespace std;

void setCursor(int x, int y) {
    COORD c = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO info = {100, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

class Cell {
    int x, y;
public:
    Cell(int x = 0, int y = 0) : x(x), y(y) {}
    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int nx) { x = nx; }
    void setY(int ny) { y = ny; }
    bool equals(const Cell& o) const { return x == o.x && y == o.y; }
};

enum Direction { UP, DOWN, LEFT, RIGHT };

class Snake {
    deque<Cell> body;
    Direction dir, nextDir;
    bool growing;
public:
    Snake(int x, int y) : dir(RIGHT), nextDir(RIGHT), growing(false) {
        body.push_back(Cell(x, y));
        body.push_back(Cell(x-1, y));
        body.push_back(Cell(x-2, y));
    }
    
    Cell getHead() const { return body.front(); }
    const deque<Cell>& getBody() const { return body; }
    
    void setDirection(Direction d) {
        if ((dir == UP && d == DOWN) || (dir == DOWN && d == UP) ||
            (dir == LEFT && d == RIGHT) || (dir == RIGHT && d == LEFT)) return;
        nextDir = d;
    }
    
    void move() {
        dir = nextDir;
        Cell head = getHead(), newHead = head;
        if (dir == UP) newHead.setY(head.getY()-1);
        else if (dir == DOWN) newHead.setY(head.getY()+1);
        else if (dir == LEFT) newHead.setX(head.getX()-1);
        else newHead.setX(head.getX()+1);
        
        body.push_front(newHead);
        if (!growing) body.pop_back();
        else growing = false;
    }
    
    void grow() { growing = true; }
    
    bool checkCollision() const {
        Cell head = getHead();
        for (size_t i = 1; i < body.size(); i++)
            if (head.equals(body[i])) return true;
        return false;
    }
    
    bool isOn(const Cell& c) const {
        for (const auto& s : body)
            if (s.equals(c)) return true;
        return false;
    }
};

class Food {
    Cell pos;
public:
    Food() : pos(0, 0) {}
    void spawn(const Snake& s, int size) {
        for (int i = 0; i < 100; i++) {
            pos = Cell(rand() % size, rand() % size);
            if (!s.isOn(pos)) break;
        }
    }
    Cell getPos() const { return pos; }
};

class Game {
    int size, score, highScore, speed;
    Snake* snake;
    Food* food;
    bool over, paused, first;
    char prev[25][25];
    
public:
    Game(int sz) : size(sz), score(0), highScore(0), speed(100),
                   over(false), paused(false), first(true) {
        snake = new Snake(sz/2, sz/2);
        food = new Food();
        food->spawn(*snake, size);
        for (int i = 0; i < 25; i++)
            for (int j = 0; j < 25; j++)
                prev[i][j] = ' ';
    }
    
    ~Game() { delete snake; delete food; }
    
    void input() {
        if (_kbhit()) {
            char k = _getch();
            if (k == -32 || k == 0) {
                k = _getch();
                if (k == 72) snake->setDirection(UP);
                else if (k == 80) snake->setDirection(DOWN);
                else if (k == 75) snake->setDirection(LEFT);
                else if (k == 77) snake->setDirection(RIGHT);
            } else {
                if (k == 'w' || k == 'W') snake->setDirection(UP);
                else if (k == 's' || k == 'S') snake->setDirection(DOWN);
                else if (k == 'a' || k == 'A') snake->setDirection(LEFT);
                else if (k == 'd' || k == 'D') snake->setDirection(RIGHT);
                else if (k == 'p' || k == 'P') paused = !paused;
                else if (k == 27) over = true;
            }
        }
    }
    
    void update() {
        if (paused || over) return;
        snake->move();
        Cell h = snake->getHead();
        if (h.getX() < 0 || h.getX() >= size || h.getY() < 0 || h.getY() >= size) {
            over = true;
            return;
        }
        if (snake->checkCollision()) {
            over = true;
            return;
        }
        if (h.equals(food->getPos())) {
            snake->grow();
            score += 10;
            if (speed > 50) speed -= 2;
            food->spawn(*snake, size);
        }
    }
    
    void draw() {
        char grid[25][25];
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                grid[i][j] = ' ';
        
        Cell fp = food->getPos();
        grid[fp.getY()][fp.getX()] = '*';
        
        const deque<Cell>& body = snake->getBody();
        for (size_t i = 0; i < body.size(); i++) {
            Cell s = body[i];
            grid[s.getY()][s.getX()] = (i == 0) ? 'O' : 'o';
        }
        
        if (first) {
            system("cls");
            cout << "==================================================\n";
            cout << "          SNAKE GAME - IT603 PROJECT             \n";
            cout << "==================================================\n";
            cout << "Score: " << score << "  |  High: " << highScore;
            if (paused) cout << " [PAUSED]";
            cout << "\n\n+";
            for (int i = 0; i < size; i++) cout << "-";
            cout << "+\n";
            for (int i = 0; i < size; i++) {
                cout << "|";
                for (int j = 0; j < size; j++) cout << grid[i][j];
                cout << "|\n";
            }
            cout << "+";
            for (int i = 0; i < size; i++) cout << "-";
            cout << "+\n\nW/A/S/D or Arrows | P=Pause | ESC=Exit\n";
            first = false;
        } else {
            setCursor(0, 3);
            cout << "Score: " << score << "  |  High: " << highScore;
            if (paused) cout << " [PAUSED]  ";
            else cout << "           ";
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    if (grid[i][j] != prev[i][j]) {
                        setCursor(j+1, i+6);
                        cout << grid[i][j];
                    }
                }
            }
        }
        
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                prev[i][j] = grid[i][j];
    }
    
    void gameOver() {
        system("cls");
        if (score > highScore) highScore = score;
        cout << "\n\n==================================================\n";
        cout << "                  GAME OVER!                     \n";
        cout << "==================================================\n\n";
        cout << "    Final Score: " << score << "\n";
        cout << "    High Score: " << highScore << "\n\n";
        cout << "    Press R to Restart or Q to Quit\n\n";
    }
    
    bool isOver() const { return over; }
    int getSpeed() const { return speed; }
    
    void reset() {
        delete snake;
        delete food;
        snake = new Snake(size/2, size/2);
        food = new Food();
        food->spawn(*snake, size);
        score = 0;
        over = paused = false;
        first = true;
        speed = 100;
        for (int i = 0; i < 25; i++)
            for (int j = 0; j < 25; j++)
                prev[i][j] = ' ';
    }
};

int main() {
    srand(time(0));
    hideCursor();
    Game game(20);
    
    cout << "==================================================\n";
    cout << "       Welcome to Snake Game - IT603!           \n";
    cout << "==================================================\n\n";
    cout << "Instructions:\n";
    cout << "- W/A/S/D or Arrow Keys to move\n";
    cout << "- Eat food (*) to grow\n";
    cout << "- Avoid walls and yourself\n";
    cout << "- P=Pause | ESC=Exit\n\n";
    cout << "Press any key to start...\n";
    _getch();
    
    while (true) {
        if (!game.isOver()) {
            game.input();
            game.update();
            game.draw();
            Sleep(game.getSpeed());
        } else {
            game.gameOver();
            char c = _getch();
            if (c == 'r' || c == 'R') game.reset();
            else if (c == 'q' || c == 'Q') break;
        }
    }
    
    return 0;
}