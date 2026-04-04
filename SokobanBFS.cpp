#include <bits/stdc++.h>
#include <windows.h>

using namespace std;

struct GameState {
    vector<string> grid;
    int px, py;
    string path; 
};

class Sokoban {
private:
    vector<string> initialGrid; 
    vector<string> grid;
    int initialPlayerX, initialPlayerY;
    int width;

    void findPlayer() {
        for (int i = 0; i < grid.size(); ++i) {
            for (int j = 0; j < grid[i].size(); ++j) {
                if (grid[i][j] == '@' || grid[i][j] == '+') {
                    initialPlayerY = i;
                    initialPlayerX = j;
                    return;
                }
            }
        }
    }

    string serializeGrid(const vector<string>& g) const {
        string s = "";
        for (const string& row : g) s += row;
        return s;
    }

    bool checkWinState(const vector<string>& g) const {
        for (const string& row : g) {
            if (row.find('$') != string::npos) return false;
        }
        return true;
    }
    // Checks if any un-goaled box is stuck in an unrecoverable corner
    bool isDeadlock(const vector<string>& g) const {
        for (int y = 0; y < g.size(); ++y) {
            for (int x = 0; x < g[y].size(); ++x) {
                if (g[y][x] == '$') { // Only check boxes that are NOT on goals
                    
                    // Check for surrounding walls
                    bool wallUp    = (y > 0 && g[y-1][x] == '#');
                    bool wallDown  = (y < g.size() - 1 && g[y+1][x] == '#');
                    bool wallLeft  = (x > 0 && g[y][x-1] == '#');
                    bool wallRight = (x < width - 1 && g[y][x+1] == '#');

                    // A box is deadlocked if it is trapped against a vertical AND horizontal wall
                    if ((wallUp || wallDown) && (wallLeft || wallRight)) {
                        return true; 
                    }
                }
            }
        }
        return false;
    }

public:
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
        initialGrid = grid;
        return true;
    }

    void resetLevel() {
        grid = initialGrid;
    }

    void render() const {
        for (const string& row : grid) cout << row << "\n";
        cout << "\n";
    }

    bool simulateMove(vector<string>& currentGrid, int& px, int& py, char dir) const {
        int dx = 0, dy = 0;
        if (dir == 'U') dy = -1;
        else if (dir == 'D') dy = 1;
        else if (dir == 'L') dx = -1;
        else if (dir == 'R') dx = 1;

        int nx = px + dx;
        int ny = py + dy;

        if (ny < 0 || ny >= currentGrid.size() || nx < 0 || nx >= width) return false;
        char nextCell = currentGrid[ny][nx];

        if (nextCell == '#') return false; 

        if (nextCell == ' ' || nextCell == '.') {
            currentGrid[py][px] = (currentGrid[py][px] == '@') ? ' ' : '.';
            currentGrid[ny][nx] = (nextCell == ' ') ? '@' : '+';
            px = nx; py = ny;
            return true;
        } 
        else if (nextCell == '$' || nextCell == '*') {
            int nnx = nx + dx;
            int nny = ny + dy;

            if (nny >= 0 && nny < currentGrid.size() && nnx >= 0 && nnx < width) {
                char cellBeyond = currentGrid[nny][nnx];
                if (cellBeyond == ' ' || cellBeyond == '.') {
                    currentGrid[nny][nnx] = (cellBeyond == ' ') ? '$' : '*';
                    currentGrid[ny][nx] = (nextCell == '$') ? '@' : '+';
                    currentGrid[py][px] = (currentGrid[py][px] == '@') ? ' ' : '.';
                    px = nx; py = ny;
                    return true;
                }
            }
        }
        return false;
    }

    string solveBFS() {
        queue<GameState> q;
        unordered_set<string> visited;

        q.push({initialGrid, initialPlayerX, initialPlayerY, ""});
        visited.insert(serializeGrid(initialGrid));

        char moves[] = {'U', 'D', 'L', 'R'};

        while (!q.empty()) {
            GameState current = q.front();
            q.pop();

            if (checkWinState(current.grid)) return current.path;

            for (char move : moves) {
                GameState nextState = current;
                
                // If the move is valid
                if (simulateMove(nextState.grid, nextState.px, nextState.py, move)) {
                    
                    // If the move pushed a box into a corner, immediately abandon this path
                    if (isDeadlock(nextState.grid)) {
                        continue; 
                    }

                    string serialized = serializeGrid(nextState.grid);
                    
                    if (visited.find(serialized) == visited.end()) {
                        visited.insert(serialized);
                        nextState.path += move;
                        q.push(nextState);
                    }
                }
            }
        }
        return "NO SOLUTION";
    }

    void playback(const string& solutionPath) {
        resetLevel();
        int px = initialPlayerX;
        int py = initialPlayerY;
        
        cout << "\n========== PLAYBACK START ==========\n\n";
        cout << "Step 0 (Initial State)\n";
        cout << "Target Path: " << solutionPath << "\n";
        render();
        cout << flush; 
        
        Sleep(1000);
        
        int step = 1;
        for (char move : solutionPath) {
            simulateMove(grid, px, py, move);
            
            cout << "------------------------------------\n";
            cout << "Step " << step++ << " / " << solutionPath.length() << " | Played Move: " << move << "\n";
            render();
            cout << flush; 
            
            Sleep(800);
        }
        
        cout << "========== LEVEL CLEAR! ==========\n";
        cout << "Total Winning Path: " << solutionPath << "\n\n";
    }
};

int main() {
    Sokoban game;
    string highlevelString = "13: ###  ####    # ####  #    # $     #    # ###   #   ##   ### #####      @    ##   ## # ## ##.####      ####  ########";
    string midlevelString = "12:      ####   #######  #  #        #  #       .# #@  ####  # #         # # # # #   # #       $ # # ####    #  ####       ";
    if (!game.loadLevel(highlevelString)) {
        cout << "Failed to load level.\n";
        return 1;
    }

    cout << "Calculating solution... (This may take a moment)\n";
    string solution = game.solveBFS();

    if (solution == "NO SOLUTION") {
        cout << "Could not find a valid path to win.\n";
    } else {
        game.playback(solution);
    }

    return 0;
}