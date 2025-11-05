#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    
    void setColor(int color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }
    
    #define COLOR_RED 12
    #define COLOR_GREEN 10
    #define COLOR_YELLOW 14
    #define COLOR_CYAN 11
    #define COLOR_WHITE 15
    #define COLOR_MAGENTA 13
    #define COLOR_BRIGHT_GREEN 10
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    
    void setColor(int color) {
        const char* colorCode;
        switch(color) {
            case 12: colorCode = "\033[91m"; break; // RED
            case 10: colorCode = "\033[92m"; break; // GREEN
            case 14: colorCode = "\033[93m"; break; // YELLOW
            case 11: colorCode = "\033[96m"; break; // CYAN
            case 15: colorCode = "\033[97m"; break; // WHITE
            case 13: colorCode = "\033[95m"; break; // MAGENTA
            default: colorCode = "\033[0m"; break;  // RESET
        }
        printf("%s", colorCode);
        fflush(stdout);
    }
    
    #define COLOR_RED 12
    #define COLOR_GREEN 10
    #define COLOR_YELLOW 14
    #define COLOR_CYAN 11
    #define COLOR_WHITE 15
    #define COLOR_MAGENTA 13
    #define COLOR_BRIGHT_GREEN 10
    
    // Global terminal settings for Linux
    struct termios orig_termios;
    bool termios_saved = false;
    
    void disableRawMode() {
        if (termios_saved) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        }
    }
    
    void enableRawMode() {
        if (!termios_saved) {
            tcgetattr(STDIN_FILENO, &orig_termios);
            atexit(disableRawMode);
            termios_saved = true;
        }
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    
    int _kbhit() {
        int byteswaiting;
        ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);
        return byteswaiting;
    }

    char _getch() {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        return ch;
    }
#endif

using namespace std;

void setCursor(int x, int y) {
#ifdef _WIN32
    COORD c = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
#else
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
#endif
}

void hideCursor() {
#ifdef _WIN32
    CONSOLE_CURSOR_INFO info = {100, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
#else
    printf("\033[?25l");
    fflush(stdout);
#endif
}

void sleepMs(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
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
    Direction getDirection() const { return dir; }
    
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
    bool over, first;
    char prev[25][25];
    
public:
    Game(int sz) : size(sz), score(0), highScore(0), speed(100),
                   over(false),first(true) {
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
            
            #ifdef _WIN32
            // Windows arrow key handling
            if (k == -32 || k == 0) {
                k = _getch();
                if (k == 72) snake->setDirection(UP);
                else if (k == 80) snake->setDirection(DOWN);
                else if (k == 75) snake->setDirection(LEFT);
                else if (k == 77) snake->setDirection(RIGHT);
            } else {
            #else
            // Linux arrow key handling
            if (k == 27) {  // ESC character
                char next1 = _getch();
                if (next1 == '[') {  // Arrow key sequence
                    char next2 = _getch();
                    if (next2 == 'A') snake->setDirection(UP);
                    else if (next2 == 'B') snake->setDirection(DOWN);
                    else if (next2 == 'C') snake->setDirection(RIGHT);
                    else if (next2 == 'D') snake->setDirection(LEFT);
                } else {
                    over = true;  // ESC key pressed
                }
            } else {
            #endif
                if (k == 'w' || k == 'W') snake->setDirection(UP);
                else if (k == 's' || k == 'S') snake->setDirection(DOWN);
                else if (k == 'a' || k == 'A') snake->setDirection(LEFT);
                else if (k == 'd' || k == 'D') snake->setDirection(RIGHT);
                #ifdef _WIN32
                else if (k == 27) over = true;
                #endif
            }
        }
    }
    
    void update() {
        if (over) return;
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
        grid[fp.getY()][fp.getX()] = '@';
        
        const deque<Cell>& body = snake->getBody();
        Direction dir = snake->getDirection();
        
        for (size_t i = 0; i < body.size(); i++) {
            Cell s = body[i];
            if (i == 0) {
                // Snake head with direction indicator
                if (dir == UP) grid[s.getY()][s.getX()] = '^';
                else if (dir == DOWN) grid[s.getY()][s.getX()] = 'v';
                else if (dir == LEFT) grid[s.getY()][s.getX()] = '<';
                else grid[s.getY()][s.getX()] = '>';
            } else {
                grid[s.getY()][s.getX()] = 'o';
            }
        }
        
        if (first) {
            clearScreen();
            setColor(COLOR_CYAN);
            cout << "+================================================+\n";
            cout << "|";
            setColor(COLOR_YELLOW);
            cout << "       SNAKE GAME - IT603 PROJECT              ";
            setColor(COLOR_CYAN);
            cout << "|\n";
            cout << "+================================================+\n";
            setColor(COLOR_GREEN);
            cout << "Score: ";
            setColor(COLOR_YELLOW);
            cout << score;
            setColor(COLOR_GREEN);
            cout << "  |  High: ";
            setColor(COLOR_YELLOW);
            cout << highScore;
            
            setColor(COLOR_WHITE);
            cout << "\n\n";
            setColor(COLOR_CYAN);
            cout << "+";
            for (int i = 0; i < size; i++) cout << "=";
            cout << "+\n";
            
            for (int i = 0; i < size; i++) {
                setColor(COLOR_CYAN);
                cout << "|";
                for (int j = 0; j < size; j++) {
                    if (grid[i][j] == '@') {
                        setColor(COLOR_RED);
                        cout << grid[i][j];
                    } else if (grid[i][j] == '^' || grid[i][j] == 'v' || 
                               grid[i][j] == '<' || grid[i][j] == '>') {
                        setColor(COLOR_BRIGHT_GREEN);
                        cout << grid[i][j];
                    } else if (grid[i][j] == 'o') {
                        setColor(COLOR_GREEN);
                        cout << grid[i][j];
                    } else {
                        setColor(COLOR_WHITE);
                        cout << grid[i][j];
                    }
                }
                setColor(COLOR_CYAN);
                cout << "|\n";
            }
            
            setColor(COLOR_CYAN);
            cout << "+";
            for (int i = 0; i < size; i++) cout << "=";
            cout << "+\n";
            setColor(COLOR_MAGENTA);
            cout << "Controls: W/A/S/D or Arrow Keys | ESC=Exit\n";
            setColor(COLOR_WHITE);
            first = false;
        } else {
            setCursor(0, 3);
            setColor(COLOR_GREEN);
            cout << "Score: ";
            setColor(COLOR_YELLOW);
            cout << score;
            setColor(COLOR_GREEN);
            cout << "  |  High: ";
            setColor(COLOR_YELLOW);
            cout << highScore;
            cout << "           ";
            setColor(COLOR_WHITE);
            
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    if (grid[i][j] != prev[i][j]) {
                        setCursor(j+1, i+6);
                        if (grid[i][j] == '@') {
                            setColor(COLOR_RED);
                            cout << grid[i][j];
                        } else if (grid[i][j] == '^' || grid[i][j] == 'v' || 
                                   grid[i][j] == '<' || grid[i][j] == '>') {
                            setColor(COLOR_BRIGHT_GREEN);
                            cout << grid[i][j];
                        } else if (grid[i][j] == 'o') {
                            setColor(COLOR_GREEN);
                            cout << grid[i][j];
                        } else {
                            setColor(COLOR_WHITE);
                            cout << grid[i][j];
                        }
                    }
                }
            }
            setColor(COLOR_WHITE);
        }
        
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                prev[i][j] = grid[i][j];
    }
    
    void gameOver() {
        sleepMs(500);  // Small delay to show final position
        clearScreen();
        hideCursor();
        if (score > highScore) highScore = score;
        cout << "\n\n";
        setColor(COLOR_RED);
        cout << "+================================================+\n";
        cout << "|                 GAME OVER!                     |\n";
        cout << "+================================================+\n\n";
        setColor(COLOR_YELLOW);
        cout << "         Final Score: ";
        setColor(COLOR_CYAN);
        cout << score << "\n";
        setColor(COLOR_YELLOW);
        cout << "         High Score:  ";
        setColor(COLOR_CYAN);
        cout << highScore << "\n\n";
        setColor(COLOR_MAGENTA);
        cout << "    Press R to Restart or Q to Quit\n\n";
        setColor(COLOR_CYAN);
        cout << "+================================================+\n";
        setColor(COLOR_WHITE);
        fflush(stdout);  // Force output to display immediately
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
        over = false;
        first = true;
        speed = 100;
        for (int i = 0; i < 25; i++)
            for (int j = 0; j < 25; j++)
                prev[i][j] = ' ';
    }
};

int main() {
    srand(time(0));
    
    #ifndef _WIN32
    enableRawMode();  // Enable raw mode for Linux terminal
    #endif
    
    hideCursor();
    Game game(20);
    
    setColor(COLOR_CYAN);
    cout << "+================================================+\n";
    cout << "|";
    setColor(COLOR_YELLOW);
    cout << "      Welcome to Snake Game - IT603!           ";
    setColor(COLOR_CYAN);
    cout << "|\n";
    cout << "+================================================+\n\n";
    setColor(COLOR_WHITE);
    cout << "Instructions:\n";
    setColor(COLOR_GREEN);
    cout << "  - Use W/A/S/D or Arrow Keys to move\n";
    cout << "  - Eat food ";
    setColor(COLOR_RED);
    cout << "(@)";
    setColor(COLOR_GREEN);
    cout << " to grow longer\n";
    cout << "  - Avoid hitting walls and yourself\n";
    cout << "  - Press ESC to Exit\n\n";
    setColor(COLOR_MAGENTA);
    cout << "Press any key to start...\n";
    setColor(COLOR_WHITE);
    _getch();
    
    while (true) {
        if (!game.isOver()) {
            game.input();
            game.update();
            game.draw();
            sleepMs(game.getSpeed());
        } else {
            game.gameOver();
            char c = _getch();
            if (c == 'r' || c == 'R') game.reset();
            else if (c == 'q' || c == 'Q') break;
        }
    }
    
    #ifndef _WIN32
    disableRawMode();  // Restore terminal on exit
    #endif
    
    setColor(COLOR_WHITE);
    return 0;
}