#include <bits/stdc++.h>
#include <windows.h>

using namespace std;

struct GameState {
    vector<string> grid;
    int px, py;
    string path; 
};

class SokobanMultiBFS {
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

    // Solves the Identity Problem for multiple boxes
    string serializeState(const vector<string>& g) const {
        string s = "";
        int px = -1, py = -1;
        vector<pair<int, int>> boxes;
        
        for (int y = 0; y < g.size(); ++y) {
            for (int x = 0; x < g[y].size(); ++x) {
                if (g[y][x] == '@' || g[y][x] == '+') {
                    px = x; py = y;
                } else if (g[y][x] == '$' || g[y][x] == '*') {
                    boxes.push_back({x, y});
                }
            }
        }
        
        // Sorting guarantees that swapped identical boxes yield the exact same state string
        sort(boxes.begin(), boxes.end());
        
        s += to_string(px) + "," + to_string(py) + "|";
        for (const auto& b : boxes) {
            s += to_string(b.first) + "," + to_string(b.second) + ";";
        }
        return s;
    }

    // Win condition: No un-goaled boxes ('$') remain on the grid
    bool checkWinState(const vector<string>& g) const {
        for (const string& row : g) {
            if (row.find('$') != string::npos) return false;
        }
        return true;
    }

    // Simple corner deadlock detector to keep BFS somewhat fast
    bool isDeadlock(const vector<string>& g) const {
        for (int y = 0; y < g.size(); ++y) {
            for (int x = 0; x < g[y].size(); ++x) {
                if (g[y][x] == '$') { 
                    bool wallUp    = (y > 0 && g[y-1][x] == '#');
                    bool wallDown  = (y < g.size() - 1 && g[y+1][x] == '#');
                    bool wallLeft  = (x > 0 && g[y][x-1] == '#');
                    bool wallRight = (x < width - 1 && g[y][x+1] == '#');

                    if ((wallUp || wallDown) && (wallLeft || wallRight)) return true; 
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

    void resetLevel() { grid = initialGrid; }
    void render() const {
        for (const string& row : grid) cout << row << "\n";
        cout << "\n";
    }
// Simulates the box movements in all the given directions
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
                
                // We only push if the space beyond is strictly empty floor or a goal.
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
        visited.insert(serializeState(initialGrid));

        char moves[] = {'U', 'D', 'L', 'R'};
        int nodesExplored = 0;

        while (!q.empty()) {
            GameState current = q.front();
            q.pop();
            nodesExplored++;

            if (checkWinState(current.grid)) {
                cout << "Nodes explored: " << nodesExplored << "\n";
                return current.path;
            }

            // Safety limit
            if (nodesExplored > 100000) return "NO SOLUTION (Timeout)";

            for (char move : moves) {
                GameState nextState = current;
                
                if (simulateMove(nextState.grid, nextState.px, nextState.py, move)) {
                    if (isDeadlock(nextState.grid)) continue; 

                    string serialized = serializeState(nextState.grid);
                    
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
        int px = initialPlayerX, py = initialPlayerY;
        
        cout << "\n========== PLAYBACK START ==========\n";
        render();
        Sleep(1000);
        
        for (char move : solutionPath) {
            simulateMove(grid, px, py, move);
            cout << "Played: " << move << "\n";
            render();
            Sleep(400); 
        }
        cout << "Total Winning Path: " << solutionPath << "\n";
    }
};

int main() {
    SokobanMultiBFS game;
    
    string multiBoxLevel = "8:#########      ## .  . ##  $$  ##  @   #########";

    if (!game.loadLevel(multiBoxLevel)) {
        cout << "Failed to load level.\n";
        return 1;
    }

    cout << "Calculating multi-box BFS solution...\n";
    string solution = game.solveBFS();

    if (solution.find("NO SOLUTION") != string::npos) {
        cout << solution << "\n";
    } else {
        game.playback(solution);
    }

    return 0;
}
