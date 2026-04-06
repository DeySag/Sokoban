#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Sokoban {
private:
    vector<string> grid;
    int playerX, playerY;
    int width;

    // Helper to locate the player '@' or '+' at the start
    void findPlayer() {
        for (int i = 0; i < grid.size(); ++i) {
            for (int j = 0; j < grid[i].size(); ++j) {
                if (grid[i][j] == '@' || grid[i][j] == '+') {
                    playerY = i;
                    playerX = j;
                    return;
                }
            }
        }
    }

    // Handles the logic of changing characters when the player steps off and onto tiles
    void updatePlayerPos(int nx, int ny) {
        //Restore the tile the player is leaving
        if (grid[playerY][playerX] == '@') grid[playerY][playerX] = ' ';
        else if (grid[playerY][playerX] == '+') grid[playerY][playerX] = '.';

        // Set the player on the new tile
        // If stepping on empty space or a space where a box just vacated
        if (grid[ny][nx] == ' ' || grid[ny][nx] == '$') grid[ny][nx] = '@';
        // If stepping on a goal or a goal where a box just vacated
        else if (grid[ny][nx] == '.' || grid[ny][nx] == '*') grid[ny][nx] = '+';

        // Update coordinates
        playerX = nx;
        playerY = ny;
    }

public:
    // Parses the "Width:LevelData" format
    bool loadLevel(const string& levelData) {
        size_t colonPos = levelData.find(':');
        if (colonPos == string::npos) return false;

        width = stoi(levelData.substr(0, colonPos));
        string data = levelData.substr(colonPos + 1);

        grid.clear();
        for (size_t i = 0; i < data.length(); i += width) {
            grid.push_back(data.substr(i, width));
        }

        findPlayer();
        return true;
    }

    void render() const {
        // ANSI escape code to clear the terminal screen and move cursor to top left
        cout << "\033[2J\033[1;1H";
        
        cout << "--- SOKOBAN ---" << "\n";
        cout << "WASD to move, ENTER to submit.\n\n";
        
        for (const string& row : grid) {
            cout << row << "\n";
        }
        cout << "\n";
    }

    bool checkWin() const {
        // If there are no raw boxes '$' left, all boxes must be on goals '*'
        for (const string& row : grid) {
            if (row.find('$') != string::npos) {
                return false;
            }
        }
        return true;
    }

    void processInput(char dir) {
        int dx = 0, dy = 0;
        
        if (dir == 'w' || dir == 'W') dy = -1;
        else if (dir == 's' || dir == 'S') dy = 1;
        else if (dir == 'a' || dir == 'A') dx = -1;
        else if (dir == 'd' || dir == 'D') dx = 1;
        else return; // Ignore invalid inputs

        int nx = playerX + dx;
        int ny = playerY + dy;

        // Prevent out of bounds just in case the map isn't enclosed by walls
        if (ny < 0 || ny >= grid.size() || nx < 0 || nx >= width) return;

        char nextCell = grid[ny][nx];

        // Collision: Wall
        if (nextCell == '#') return; 

        // Collision: Empty Space or Goal
        if (nextCell == ' ' || nextCell == '.') {
            updatePlayerPos(nx, ny);
        } 
        // Collision: Box or Box on Goal
        else if (nextCell == '$' || nextCell == '*') {
            int nnx = nx + dx; // The space *behind* the box
            int nny = ny + dy;

            if (nny >= 0 && nny < grid.size() && nnx >= 0 && nnx < width) {
                char cellBeyondBox = grid[nny][nnx];
                
                // Can only push if the space behind the box is empty or a goal
                if (cellBeyondBox == ' ' || cellBeyondBox == '.') {
                    
                    // Move the box first
                    if (cellBeyondBox == ' ') grid[nny][nnx] = '$';
                    else if (cellBeyondBox == '.') grid[nny][nnx] = '*';

                    // Then move the player into the space the box just left
                    updatePlayerPos(nx, ny);
                }
                // If it's a wall (#) or another box ($ or *), do nothing.
            }
        }
    }
};

int main() {
    Sokoban game;
    
    string levelString = "12:      ####   #######  #  #        #  #       .# #@  ####  # #         # # # # #   # #       $ # # ####    #  ####       "; 
    
    if (!game.loadLevel(levelString)) {
        cout << "Failed to load level.\n";
        return 1;
    }

    // Game Loop
    while (true) {
        game.render();
        
        if (game.checkWin()) {
            cout << "Level Clear!\n";
            break;
        }

        cout << "Move (WASD) or 'Q' to quit: ";
        char move;
        cin >> move;
        
        // Add termination condition
        if (move == 'q' || move == 'Q') {
            cout << "Game exited by player.\n";
            break;
        }
        
        game.processInput(move);
    }

    return 0;
}
