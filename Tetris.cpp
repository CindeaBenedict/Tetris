#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>

using namespace std;

const int width = 20;
const int height = 20;

struct Point {
    int x, y;
};

class Tetris {
public:
    Tetris() : score(0), holdPieceAvailable(true), gameSpeed(100), holdColor(0), speedMultiplier(1.0) { // Even faster game speed
        srand(time(0));
        board.resize(height, vector<int>(width, 0));
        generateNextPiece();
        newPiece();
    }

    void generateNextPiece() {
        int n = rand() % 7;
        switch(n) {
            case 0: // I
                nextPiece = {{0,1}, {1,1}, {2,1}, {3,1}};
                nextColor = 1;
                break;
            case 1: // J
                nextPiece = {{0,1}, {1,1}, {2,1}, {2,0}};
                nextColor = 2;
                break;
            case 2: // L
                nextPiece = {{0,0}, {0,1}, {1,1}, {2,1}};
                nextColor = 3;
                break;
            case 3: // O
                nextPiece = {{1,0}, {1,1}, {2,0}, {2,1}};
                nextColor = 4;
                break;
            case 4: // S
                nextPiece = {{0,1}, {1,1}, {1,0}, {2,0}};
                nextColor = 5;
                break;
            case 5: // T
                nextPiece = {{0,1}, {1,0}, {1,1}, {2,1}};
                nextColor = 6;
                break;
            case 6: // Z
                nextPiece = {{0,0}, {1,0}, {1,1}, {2,1}};
                nextColor = 7;
                break;
        }
    }

    void newPiece() {
        currentPiece = nextPiece;
        currentColor = nextColor;
        currentX = width / 2 - 1;
        currentY = 0;
        generateNextPiece();
    }

    bool checkCollision(int dx, int dy) {
        for (auto p : currentPiece) {
            int newX = currentX + p.x + dx;
            int newY = currentY + p.y + dy;
            if (newX < 0 || newX >= width || newY >= height || (newY >= 0 && board[newY][newX])) {
                return true;
            }
        }
        return false;
    }

    bool checkRotationCollision(const vector<Point>& rotatedPiece) {
        for (auto p : rotatedPiece) {
            int newX = currentX + p.x;
            int newY = currentY + p.y;
            if (newX < 0 || newX >= width || newY >= height || (newY >= 0 && board[newY][newX])) {
                return true;
            }
        }
        return false;
    }

    void mergePiece() {
        for (auto p : currentPiece) {
            if (currentY + p.y >= 0) {
                board[currentY + p.y][currentX + p.x] = currentColor;
            }
        }
    }

    void clearLines() {
        for (int y = height - 1; y >= 0; --y) {
            bool fullLine = true;
            for (int x = 0; x < width; ++x) {
                if (!board[y][x]) {
                    fullLine = false;
                    break;
                }
            }
            if (fullLine) {
                score += 100;
                for (int yy = y; yy > 0; --yy) {
                    for (int x = 0; x < width; ++x) {
                        board[yy][x] = board[yy - 1][x];
                    }
                }
                for (int x = 0; x < width; ++x) {
                    board[0][x] = 0;
                }
                ++y;
            }
        }
    }

    void rotatePiece() {
        vector<Point> rotated;
        for (auto p : currentPiece) {
            rotated.push_back({-p.y, p.x});
        }
        if (!checkRotationCollision(rotated)) {
            currentPiece = rotated;
        }
    }

    void movePiece(int dx, int dy) {
        if (!checkCollision(dx, dy)) {
            currentX += dx;
            currentY += dy;
        }
    }

    void holdCurrentPiece() {
        if (!holdPieceAvailable) return;
        if (holdPiece.empty()) {
            holdPiece = currentPiece;
            holdColor = currentColor;
            newPiece();
        } else {
            swap(currentPiece, holdPiece);
            swap(currentColor, holdColor);
            currentX = width / 2 - 1;
            currentY = 0;
        }
        holdPieceAvailable = false;
    }

    void dropPiece() {
        while (!checkCollision(0, 1)) {
            currentY++;
        }
    }

    bool step() {
        if (!checkCollision(0, 1)) {
            ++currentY;
        } else {
            mergePiece();
            clearLines();
            newPiece();
            holdPieceAvailable = true;
            if (checkCollision(0, 0)) {
                return false; // Game over
            }
        }
        return true;
    }

    void draw() {
        clear();
        // Draw border
        for (int y = 0; y < height + 2; ++y) {
            for (int x = 0; x < width + 2; ++x) {
                if (y == 0 || y == height + 1 || x == 0 || x == width + 1) {
                    mvprintw(y, x, "#");
                }
            }
        }
        // Draw board
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (board[y][x]) {
                    attron(COLOR_PAIR(board[y][x]) | A_BOLD);
                    mvprintw(y + 1, x + 1, "#");
                    attroff(COLOR_PAIR(board[y][x]) | A_BOLD);
                }
            }
        }
        // Draw current piece
        for (auto p : currentPiece) {
            attron(COLOR_PAIR(currentColor) | A_BOLD);
            mvprintw(currentY + p.y + 1, currentX + p.x + 1, "#");
            attroff(COLOR_PAIR(currentColor) | A_BOLD);
        }
        // Draw hold piece
        mvprintw(0, width + 5, "Hold:");
        if (!holdPiece.empty()) {
            for (auto p : holdPiece) {
                attron(COLOR_PAIR(holdColor) | A_BOLD);
                mvprintw(p.y + 1, width + 5 + p.x, "#");
                attroff(COLOR_PAIR(holdColor) | A_BOLD);
            }
        }
        // Draw next piece
        mvprintw(0, width + 15, "Next:");
        for (auto p : nextPiece) {
            attron(COLOR_PAIR(nextColor) | A_BOLD);
            mvprintw(p.y + 1, width + 15 + p.x, "#");
            attroff(COLOR_PAIR(nextColor) | A_BOLD);
        }
        // Draw score
        mvprintw(height + 3, 0, "Score: %d", score);
        mvprintw(height + 4, 0, "Press 'r' to restart, 'q' to quit, 'c' to hold piece");
        refresh();
    }

    void restart() {
        score = 0;
        board.clear();
        board.resize(height, vector<int>(width, 0));
        holdPiece.clear();
        holdPieceAvailable = true;
        generateNextPiece();
        newPiece();
    }

    void updateSpeed() {
        gameSpeed = 100 - (score / 5000) * 10; // Even faster base speed
        if (gameSpeed < 20) gameSpeed = 20;
    }

    int getScore() const {
        return score;
    }

    int getSpeed() const {
        return gameSpeed;
    }

    double getSpeedMultiplier() const {
        return speedMultiplier;
    }

    void setSpeedMultiplier(double multiplier) {
        speedMultiplier = multiplier;
    }

private:
    vector<vector<int>> board;
    vector<Point> currentPiece;
    vector<Point> nextPiece;
    vector<Point> holdPiece;
    int currentX, currentY, currentColor, nextColor, holdColor;
    int score;
    int gameSpeed;
    double speedMultiplier;
    bool holdPieceAvailable;
};

int main() {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    start_color();

    // Initialize colors
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);
    // Create brighter color pairs
    init_pair(8, COLOR_CYAN, COLOR_WHITE);
    init_pair(9, COLOR_BLUE, COLOR_WHITE);
    init_pair(10, COLOR_WHITE, COLOR_BLUE);
    init_pair(11, COLOR_YELLOW, COLOR_BLUE);
    init_pair(12, COLOR_GREEN, COLOR_WHITE);
    init_pair(13, COLOR_MAGENTA, COLOR_WHITE);
    init_pair(14, COLOR_RED, COLOR_WHITE);

    Tetris tetris;
    bool running = true;
    int tickCounter = 0;

    while (running) {
        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                tetris.movePiece(-1, 0);
                break;
            case KEY_RIGHT:
                tetris.movePiece(1, 0);
                break;
            case KEY_UP:
                tetris.rotatePiece();
                break;
            case KEY_DOWN:
                tetris.setSpeedMultiplier(0.2); // Increase falling speed significantly
                break;
            case ' ':
                tetris.dropPiece();
                break;
            case 'c':
                tetris.holdCurrentPiece();
                break;
            case 'r':
                tetris.restart();
                break;
            case 'q':
                running = false;
                break;
        }

        if (ch != KEY_DOWN) {
            tetris.setSpeedMultiplier(1.0); // Reset speed multiplier
        }

        if (tickCounter >= tetris.getSpeed() * tetris.getSpeedMultiplier()) {
            if (!tetris.step()) {
                running = false;
            }
            tetris.updateSpeed();
            tickCounter = 0;
        }

        tetris.draw();
        tickCounter++;
        napms(10); // Small delay to control speed
    }

    endwin();
    cout << "Game Over! Final Score: " << tetris.getScore() << endl;

    return 0;
}
