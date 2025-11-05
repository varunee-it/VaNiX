#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <fstream>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define CLEAR "cls"
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#define CLEAR "clear"

// Global terminal settings for Linux
struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;  // Small timeout for reading
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int kbhit() {
    int byteswaiting;
    ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);
    return byteswaiting > 0;
}

int getch() {
    char ch;
    if (read(STDIN_FILENO, &ch, 1) == 1)
        return ch;
    return -1;
}

// Blocking getch for menus (waits for actual key press)
int getchBlocking() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    newt.c_cc[VMIN] = 1;  // Wait for at least 1 character
    newt.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    char ch;
    read(STDIN_FILENO, &ch, 1);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void Sleep(int ms) { usleep(ms * 1000); }
#endif

using namespace std;

// ---------- Enhanced Color Scheme ----------
#define COLOR_RESET     "\033[0m"
#define COLOR_BG        "\033[48;5;234m"
#define COLOR_SNAKE     "\033[38;5;46m"
#define COLOR_HEAD      "\033[38;5;82;1m"
#define COLOR_FOOD      "\033[38;5;196m"
#define COLOR_WALL      "\033[48;5;240m"
#define COLOR_SCORE     "\033[38;5;226m"
#define COLOR_HIGHSCORE "\033[38;5;214m"
#define COLOR_TITLE     "\033[38;5;51;1m"
#define COLOR_TEXT      "\033[38;5;255m"
#define COLOR_CONTROLS  "\033[38;5;117m"
#define COLOR_GAMEOVER  "\033[38;5;203;1m"
#define COLOR_BOX       "\033[38;5;45m"
#define COLOR_KEY       "\033[38;5;228m"
#define COLOR_ARROW     "\033[38;5;213m"
#define COLOR_GREEN_TEXT "\033[38;5;84m"
#define COLOR_PINK_TEXT "\033[38;5;212m"

// ---------- Core Structures ----------
struct Pos {
    int x, y;
    Pos(int x=0, int y=0) : x(x), y(y) {}
    bool operator==(const Pos& p) const { return x==p.x && y==p.y; }
};

enum Dir { UP, DOWN, LEFT, RIGHT };

// ---------- High Score Manager ----------
class HighScoreManager {
private:
    string filename;
    
public:
    HighScoreManager() : filename("snake_highscore.txt") {}
    
    int loadHighScore() {
        ifstream file(filename);
        int highScore = 0;
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }
        return highScore;
    }
    
    void saveHighScore(int score) {
        ofstream file(filename);
        if (file.is_open()) {
            file << score;
            file.close();
        }
    }
    
    bool updateHighScore(int currentScore, int& currentHighScore) {
        if (currentScore > currentHighScore) {
            currentHighScore = currentScore;
            saveHighScore(currentHighScore);
            return true;
        }
        return false;
    }
};

// ---------- Snake ----------
class Snake {
    deque<Pos> body;
    Dir d, nd;
    bool growFlag;
public:
    Snake(int x, int y) : d(RIGHT), nd(RIGHT), growFlag(false) {
        body.push_back(Pos(x, y));
        body.push_back(Pos(x - 1, y));
        body.push_back(Pos(x - 2, y));
    }

    Pos head() const { return body.front(); }
    const deque<Pos>& getBody() const { return body; }
    Dir getDirection() const { return d; }

    void setDir(Dir newDir) {
        if ((d==UP && newDir==DOWN) || (d==DOWN && newDir==UP) ||
            (d==LEFT && newDir==RIGHT) || (d==RIGHT && newDir==LEFT)) return;
        nd = newDir;
    }

    void move() {
        d = nd;
        Pos h = head();
        if (d == UP) h.y--;
        else if (d == DOWN) h.y++;
        else if (d == LEFT) h.x--;
        else if (d == RIGHT) h.x++;

        body.push_front(h);
        if (!growFlag) body.pop_back();
        else growFlag = false;
    }

    void grow() { growFlag = true; }

    bool hitSelf() const {
        Pos h = head();
        for (size_t i = 1; i < body.size(); i++)
            if (body[i] == h) return true;
        return false;
    }

    bool onPos(Pos p) const {
        for (auto& b : body)
            if (b == p) return true;
        return false;
    }
};

// ---------- Food ----------
class Food {
    Pos p;
public:
    void spawn(const Snake& s, int size) {
        while (true) {
            p = Pos(rand() % size, rand() % size);
            if (!s.onPos(p)) break;
        }
    }
    Pos pos() const { return p; }
};

// ---------- Game ----------
class Game {
    int size, sc, hi, sp;
    Snake* sn;
    Food* fd;
    bool ov;
    HighScoreManager hsManager;
    bool newHighScore;
    
public:
    Game(int s) : size(s), sc(0), hi(0), ov(false), newHighScore(false) {
#ifdef _WIN32
        sp = 15;
#else
        sp = 150;
#endif
        sn = new Snake(s / 2, s / 2);
        fd = new Food();
        fd->spawn(*sn, size);
        hi = hsManager.loadHighScore();
    }

    ~Game() { delete sn; delete fd; }

    void input() {
        if (!kbhit()) return;
        int key = getch();
        if (key == -1) return;

#ifdef _WIN32
        if (key == 27) {
            ov = true;
            return;
        }
        else if (key == 0 || key == 224) {
            key = getch();
            if (key == 72) sn->setDir(UP);
            else if (key == 80) sn->setDir(DOWN);
            else if (key == 75) sn->setDir(LEFT);
            else if (key == 77) sn->setDir(RIGHT);
        } else
#else
        if (key == 27) {
            // Read next character immediately without checking kbhit
            int next = getch();
            if (next == '[') {
                // This is an arrow key sequence
                int arrow = getch();
                if (arrow == 'A') sn->setDir(UP);
                else if (arrow == 'B') sn->setDir(DOWN);
                else if (arrow == 'C') sn->setDir(RIGHT);
                else if (arrow == 'D') sn->setDir(LEFT);
            } else if (next == -1) {
                // Just ESC key pressed (no following character)
                ov = true;
            }
            return;
        } else
#endif
        {
            if (key=='w'||key=='W') sn->setDir(UP);
            else if (key=='s'||key=='S') sn->setDir(DOWN);
            else if (key=='a'||key=='A') sn->setDir(LEFT);
            else if (key=='d'||key=='D') sn->setDir(RIGHT);
        }
    }

    void update() {
        if (ov) return;
        sn->move();
        Pos h = sn->head();

        if (h.x < 0 || h.x >= size || h.y < 0 || h.y >= size) { ov = true; return; }
        if (sn->hitSelf()) { ov = true; return; }
        if (h == fd->pos()) {
            sn->grow();
            sc += 10;
            fd->spawn(*sn, size);
        }
    }

    void draw() const {
        cout << "\033[H";
        
        cout << COLOR_BG << COLOR_TEXT << "  =====================================" << COLOR_RESET << "\n";
        cout << COLOR_BG << COLOR_TEXT << "                " << COLOR_TITLE << "🐍 SNAKE GAME 🐍" << COLOR_TEXT << "       " << COLOR_RESET << "\n";
        cout << COLOR_BG << "            " << COLOR_SCORE << "Score: " << sc << COLOR_TEXT << "   " 
             << COLOR_HIGHSCORE << "High: " << hi << COLOR_TEXT << "      " << COLOR_RESET << "\n";
        cout << COLOR_BG << COLOR_TEXT << "  =====================================" << COLOR_RESET << "\n\n";

        cout << "  " << COLOR_WALL;
        for (int i=0;i<size*2+2;i++) cout << " ";
        cout << COLOR_RESET << "\n";

        for (int y=0;y<size;y++) {
            cout << "  " << COLOR_WALL << " " << COLOR_RESET;
            
            for (int x=0;x<size;x++) {
                Pos current(x, y);
                bool isSnake = false;
                bool isHead = false;
                bool isFood = false;
                
                if (sn->head() == current) {
                    isHead = true;
                } else if (fd->pos() == current) {
                    isFood = true;
                } else {
                    for (auto& b : sn->getBody()) {
                        if (b == current) {
                            isSnake = true;
                            break;
                        }
                    }
                }
                
                if (isHead) {
                    cout << "🐍";
                } else if (isSnake) {
                    cout << "🟢";
                } else if (isFood) {
                    cout << "🍎";
                } else {
                    cout << COLOR_BG << "  " << COLOR_RESET;
                }
            }
            
            cout << COLOR_WALL << " " << COLOR_RESET << "\n";
        }

        cout << "  " << COLOR_WALL;
        for (int i=0;i<size*2+2;i++) cout << " ";
        cout << COLOR_RESET << "\n";
        cout << COLOR_BG << COLOR_TEXT << "\n  =====================================" << COLOR_RESET << "\n";
        cout << COLOR_BG << COLOR_CONTROLS << "               Exit: " << COLOR_TEXT << "ESC" << COLOR_RESET << "\n";
        cout << COLOR_BG << COLOR_TEXT << "  =====================================" << COLOR_RESET << "\n";
        
        cout.flush();
    }

    bool over() const { return ov; }
    int speed() const { return sp; }

    void drawControlBox() const {
        cout << COLOR_BOX << "  +----------------------------------------+" << COLOR_RESET << "\n";
        cout << COLOR_BOX << "  |" << COLOR_TITLE << "              CONTROLS               " << COLOR_BOX << "   |" << COLOR_RESET << "\n";
        cout << COLOR_BOX << "  +----------------------------------------+" << COLOR_RESET << "\n";
        cout << COLOR_BOX << "  |                  " << COLOR_KEY << "W" << COLOR_BOX << "                     |" << COLOR_RESET << "\n";
        cout << COLOR_BOX << "  |                " << COLOR_KEY << "A S D" << COLOR_BOX << "                   |" << COLOR_RESET << "\n";
        cout << COLOR_BOX <<"  |                                        |\n";
        cout << COLOR_BOX << "  |                  " << COLOR_ARROW << "^" << COLOR_BOX << "                     |" << COLOR_RESET << "\n";
        cout << COLOR_BOX << "  |                " << COLOR_ARROW << "< v >" << COLOR_BOX << "                   |" << COLOR_RESET << "\n";
        cout << COLOR_BOX << "  +----------------------------------------+" << COLOR_RESET << "\n\n";
        cout << "     " << COLOR_GREEN_TEXT << "🍎 Eat apples to grow and score!" << COLOR_RESET << "\n";
        cout << "     " << COLOR_GREEN_TEXT << "🐍 Avoid walls and yourself!" << COLOR_RESET << "\n\n";
    }

    void end() {
        system(CLEAR);
        newHighScore = hsManager.updateHighScore(sc, hi);

        cout << COLOR_BG << "\n\n";
        cout << "    " << COLOR_TEXT << "=====================================" << COLOR_RESET << "\n";
        cout << "            " << COLOR_GAMEOVER << "     GAME OVER! 💀" << COLOR_RESET << "        \n";
        cout << "    " << COLOR_TEXT << "=====================================" << COLOR_RESET << "\n\n";
        cout << COLOR_TEXT << "              Final Score: " << COLOR_SCORE << sc << COLOR_RESET << "\n";
        cout << COLOR_TEXT << "              High Score:  " << COLOR_HIGHSCORE << hi << COLOR_RESET << "\n\n";
        if (newHighScore && sc > 0) 
            cout << "         " << COLOR_HIGHSCORE << "  🏆 NEW HIGH SCORE! 🏆" << COLOR_RESET << "\n\n";
        
        cout << COLOR_TEXT << "          [" << COLOR_CONTROLS << "R" << COLOR_TEXT << "] Restart  |  [" 
             << COLOR_CONTROLS << "Q" << COLOR_TEXT << "] Quit" << COLOR_RESET << "\n";
        cout << COLOR_TEXT << " \n         [Any other key]  Main Menu" << COLOR_RESET << "\n";
        cout << "    " << COLOR_TEXT << "=====================================" << COLOR_RESET << "\n\n";
        cout << COLOR_RESET;
    }

    void welcome() const {
        system(CLEAR);
        cout << COLOR_BG << "\n\n";
        cout << "    " << COLOR_TEXT << "=====================================" << COLOR_RESET << "\n";
        cout << "               " << COLOR_TITLE << "🐍 SNAKE GAME 🐍" << COLOR_RESET << "        \n";
        cout << "    " << COLOR_TEXT << "=====================================" << COLOR_RESET << "\n\n";
        
        cout << "        " << COLOR_HIGHSCORE << "      🏆 High Score: " << hi << COLOR_RESET << "\n\n";
        
        drawControlBox();
        
        cout << COLOR_PINK_TEXT << "     Press any key to start playing..." << COLOR_RESET << "\n\n";
        cout << COLOR_RESET;
    }

    void reset() {
        delete sn;
        delete fd;
        sn = new Snake(size/2, size/2);
        fd = new Food();
        fd->spawn(*sn, size);
        sc = 0;
#ifdef _WIN32
        sp = 15;
#else
        sp = 150;
#endif
        ov = false;
        newHighScore = false;
        
        system(CLEAR);
        cout << "\033[2J";
        cout << "\033[H";
    }
};

// ---------- Main ----------
int main() {
    srand(time(0));
    
#ifndef _WIN32
    enableRawMode();
#endif
    
    Game g(20);
    bool showWelcome = true;
    
    while (true) {
        if (showWelcome) {
            g.welcome();
#ifdef _WIN32
            getch();
#else
            getchBlocking();  // Use blocking getch for menu
#endif
            showWelcome = false;
        }

        cout << "\033[2J";
        cout << "\033[?25l";

        while (!g.over()) {
            g.input();
            g.update();
            g.draw();
            Sleep(g.speed());
        }

        cout << "\033[?25h";
        
        g.end();
#ifdef _WIN32
        int c = getch();
#else
        int c = getchBlocking();  // Use blocking getch for game over menu
#endif
        if (c=='r'||c=='R') {
            g.reset();
            showWelcome = false;
        } else if (c=='q'||c=='Q') {
            cout << "         🎮 Thanks for playing! 🐍\n\n";
            break;
        } else {
            showWelcome = true;
        }
    }
    
#ifndef _WIN32
    disableRawMode();
#endif
     
    return 0;
}