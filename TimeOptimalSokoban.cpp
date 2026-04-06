#include <bits/stdc++.h>
#include <windows.h>

using namespace std;

struct GameState {
    vector<string> grid;
    int px, py;
    string path;
    int gCost;  // Cost from start (number of moves)
    int hCost;  // Heuristic cost to goal
    
    int fCost() const {
        // We multiply the heuristic by 2.5 to make the search much faster
        return gCost + (hCost * 2.5); 
    }
};

// For priority queue ordering (lower f-cost = higher priority)
struct CompareGameState {
    bool operator()(const GameState& a, const GameState& b) const {
        if (a.fCost() != b.fCost()) {
            return a.fCost() > b.fCost();  // Lower f-cost has higher priority
        }
        return a.path.length() > b.path.length();  // Tiebreaker: prefer shorter paths
    }
};

class SokobanAStar {
private:
    vector<string> initialGrid;
    vector<string> grid;
    int initialPlayerX, initialPlayerY;
    int width;
    vector<pair<int, int>> goalPositions;
    vector<vector<bool>> deadSquares; // Pre-calculated dead squares

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

    void findGoals() {
        goalPositions.clear();
        for (int i = 0; i < grid.size(); ++i) {
            for (int j = 0; j < grid[i].size(); ++j) {
                if (grid[i][j] == '.' || grid[i][j] == '+') {
                    goalPositions.push_back({j, i});
                }
            }
        }
    }

    // Minified Board State for Transposition Table
    string serializeState(const vector<string>& g) const {
        string s = "";
        int px = -1, py = -1;
        vector<pair<int, int>> boxes;
        
        // Extract only the dynamic elements (Player and Boxes)
        for (int y = 0; y < g.size(); ++y) {
            for (int x = 0; x < g[y].size(); ++x) {
                if (g[y][x] == '@' || g[y][x] == '+') {
                    px = x; py = y;
                } else if (g[y][x] == '$' || g[y][x] == '*') {
                    boxes.push_back({x, y});
                }
            }
        }
        
        // Sort boxes so identical box configurations yield the exact same string
        // regardless of the order we found them in
        sort(boxes.begin(), boxes.end());
        
        // Build a tiny string: "px,py|boxX,boxY;boxX,boxY;"
        s += to_string(px) + "," + to_string(py) + "|";
        for (const auto& b : boxes) {
            s += to_string(b.first) + "," + to_string(b.second) + ";";
        }
        return s;
    }

    // Pre-calculate all simple deadlocks using a pull-search
    void precalculateDeadlocks() {
        // Assume everywhere is a dead square initially
        deadSquares.assign(grid.size(), vector<bool>(width, true));
        queue<pair<int, int>> q;
        
        // Goals are never dead squares. Start pull-BFS from all goals.
        for (const auto& goal : goalPositions) {
            deadSquares[goal.second][goal.first] = false;
            q.push(goal);
        }
        
        int dx[] = {0, 0, -1, 1};
        int dy[] = {-1, 1, 0, 0};
        
        while (!q.empty()) {
            pair<int, int> currentPos = q.front();
            int cx = currentPos.first;
            int cy = currentPos.second;
            q.pop();
            
            for (int i = 0; i < 4; ++i) {
                int nx = cx + dx[i]; // Box's previous position (if pulled)
                int ny = cy + dy[i];
                
                int px = cx + 2 * dx[i]; // Player's position needed to push the box
                int py = cy + 2 * dy[i];
                
                // Bounds check
                if (nx >= 0 && nx < width && ny >= 0 && ny < grid.size() &&
                    px >= 0 && px < width && py >= 0 && py < grid.size()) {
                    
                    // If the box could have been legally pushed from (nx, ny) by a player at (px, py)
                    if (initialGrid[ny][nx] != '#' && initialGrid[py][px] != '#') {
                        // If we haven't visited this square yet, mark it safe and queue it
                        if (deadSquares[ny][nx]) {
                            deadSquares[ny][nx] = false;
                            q.push({nx, ny});
                        }
                    }
                }
            }
        }
    }

    // Extended Move Generator (Macro Moves)
    vector<GameState> generateMacroMoves(const GameState& current) const {
        vector<GameState> nextStates;
        
        // Queue stores: {{x, y}, path_taken_to_get_here}
        queue<pair<pair<int, int>, string>> q; 
        vector<vector<bool>> visited(grid.size(), vector<bool>(width, false));
        
        q.push({{current.px, current.py}, ""});
        visited[current.py][current.px] = true;
        
        int dx[] = {0, 0, -1, 1};
        int dy[] = {-1, 1, 0, 0};
        char dirName[] = {'U', 'D', 'L', 'R'};
        
        while (!q.empty()) {
            pair<pair<int, int>, string> frontNode = q.front();
            int cx = frontNode.first.first;
            int cy = frontNode.first.second;
            string walkPath = frontNode.second;
            q.pop();
            
            for (int i = 0; i < 4; ++i) {
                int nx = cx + dx[i];
                int ny = cy + dy[i];
                
                if (nx < 0 || nx >= width || ny < 0 || ny >= current.grid.size()) continue;
                
                char cell = current.grid[ny][nx];
                
                // 1. If it's empty space, player can walk there. Queue it for the internal BFS.
                if (cell == ' ' || cell == '.') {
                    if (!visited[ny][nx]) {
                        visited[ny][nx] = true;
                        q.push({{nx, ny}, walkPath + dirName[i]});
                    }
                } 
                // 2. If it's a box, check if we can push it!
                else if (cell == '$' || cell == '*') {
                    int pushX = nx + dx[i];
                    int pushY = ny + dy[i];
                    
                    if (pushX >= 0 && pushX < width && pushY >= 0 && pushY < current.grid.size()) {
                        char beyond = current.grid[pushY][pushX];
                        
                        // If the space behind the box is empty or a goal, it's a valid push!
                        if (beyond == ' ' || beyond == '.') {
                            GameState nextState = current;
                            
                            // Erase player from the original starting position
                            nextState.grid[current.py][current.px] = (nextState.grid[current.py][current.px] == '+') ? '.' : ' ';
                            
                            // Move the box to its new position
                            nextState.grid[pushY][pushX] = (beyond == '.') ? '*' : '$';
                            
                            // Move the player into the box's old position
                            nextState.grid[ny][nx] = (cell == '*') ? '+' : '@';
                            
                            // Update State data
                            nextState.px = nx;
                            nextState.py = ny;
                            nextState.path += walkPath + dirName[i]; 
                            nextState.gCost += walkPath.length() + 1; // Real step count
                            
                            nextStates.push_back(nextState);
                        }
                    }
                }
            }
        }
        return nextStates;
    }

    bool checkWinState(const vector<string>& g) const {
        for (const string& row : g) {
            if (row.find('$') != string::npos) return false;
        }
        return true;
    }

    // Manhattan distance heuristic
    // Calculates minimum cost to move all boxes to goal positions
    int calculateHeuristic(const vector<string>& g) const {
        vector<pair<int, int>> boxes;
        vector<pair<int, int>> availableGoals = goalPositions;

        // Find all box positions
        for (int y = 0; y < g.size(); ++y) {
            for (int x = 0; x < g[y].size(); ++x) {
                if (g[y][x] == '$' || g[y][x] == '*') {
                    boxes.push_back({x, y});
                }
            }
        }

        if (boxes.empty()) return 0;

        // Calculate minimum cost using Hungarian-like approach
        // For simplicity, we use a greedy assignment with minimum distance
        int totalCost = 0;
        vector<bool> usedGoals(availableGoals.size(), false);

        for (const auto& box : boxes) {
            int minDist = INT_MAX;
            int bestGoal = -1;

            for (int i = 0; i < availableGoals.size(); ++i) {
                if (!usedGoals[i]) {
                    int dist = abs(box.first - availableGoals[i].first) +
                               abs(box.second - availableGoals[i].second);
                    if (dist < minDist) {
                        minDist = dist;
                        bestGoal = i;
                    }
                }
            }

            if (bestGoal != -1) {
                usedGoals[bestGoal] = true;
                totalCost += minDist;
            }
        }

        // Add cost to move player to closest box (if any)
        if (!boxes.empty()) {
            int minPlayerDist = INT_MAX;
            for (const auto& box : boxes) {
                int dist = abs(initialPlayerX - box.first) +
                           abs(initialPlayerY - box.second);
                minPlayerDist = min(minPlayerDist, dist);
            }
            totalCost += minPlayerDist / 2;  // Approximate cost to reach a box
        }

        return totalCost;
    }

    // Checks if any un-goaled box is stuck in an unrecoverable corner
    bool isDeadlock(const vector<string>& g) const {
        for (int y = 0; y < g.size(); ++y) {
            for (int x = 0; x < g[y].size(); ++x) {
                // Instantly know if a box was pushed onto a dead square
                if (g[y][x] == '$' && deadSquares[y][x]) {
                    return true;
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
        findGoals();
        initialGrid = grid;
        
        precalculateDeadlocks(); // Build the deadlock table before searching

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
        if (dir == 'U')
            dy = -1;
        else if (dir == 'D')
            dy = 1;
        else if (dir == 'L')
            dx = -1;
        else if (dir == 'R')
            dx = 1;

        int nx = px + dx;
        int ny = py + dy;

        if (ny < 0 || ny >= currentGrid.size() || nx < 0 || nx >= width)
            return false;
        char nextCell = currentGrid[ny][nx];

        if (nextCell == '#')
            return false;

        if (nextCell == ' ' || nextCell == '.') {
            currentGrid[py][px] = (currentGrid[py][px] == '@') ? ' ' : '.';
            currentGrid[ny][nx] = (nextCell == ' ') ? '@' : '+';
            px = nx;
            py = ny;
            return true;
        } else if (nextCell == '$' || nextCell == '*') {
            int nnx = nx + dx;
            int nny = ny + dy;

            if (nny >= 0 && nny < currentGrid.size() && nnx >= 0 && nnx < width) {
                char cellBeyond = currentGrid[nny][nnx];
                if (cellBeyond == ' ' || cellBeyond == '.') {
                    currentGrid[nny][nnx] = (cellBeyond == ' ') ? '$' : '*';
                    currentGrid[ny][nx] = (nextCell == '$') ? '@' : '+';
                    currentGrid[py][px] = (currentGrid[py][px] == '@') ? ' ' : '.';
                    px = nx;
                    py = ny;
                    return true;
                }
            }
        }
        return false;
    }

    string solveAStar() {
        priority_queue<GameState, vector<GameState>, CompareGameState> openSet;
        unordered_set<string> closedSet;

        GameState initial;
        initial.grid = initialGrid;
        initial.px = initialPlayerX;
        initial.py = initialPlayerY;
        initial.path = "";
        initial.gCost = 0;
        initial.hCost = calculateHeuristic(initialGrid);

        openSet.push(initial);
        int nodesExplored = 0;

        while (!openSet.empty()) {
            GameState current = openSet.top();
            openSet.pop();

            nodesExplored++;

            string serialized = serializeState(current.grid);
            if (closedSet.find(serialized) != closedSet.end()) {
                continue;
            }
            closedSet.insert(serialized);

            if (checkWinState(current.grid)) {
                cout << "Nodes explored with A*: " << nodesExplored << "\n";
                return current.path;
            }

            // Limit search to prevent excessive computation
            if (nodesExplored > 100000) {
                cout << "Search limit exceeded (>100000 nodes)\n";
                break;
            }

            // Use the Extended Move Generator instead of stepping blindly
            vector<GameState> macroMoves = generateMacroMoves(current);
            
            for (GameState& nextState : macroMoves) {
                
                // 1. Instant Deadlock check
                if (isDeadlock(nextState.grid)) {
                    continue;
                }

                // 2. Transposition Table check
                string nextSerialized = serializeState(nextState.grid);
                
                if (closedSet.find(nextSerialized) == closedSet.end()) {
                    nextState.hCost = calculateHeuristic(nextState.grid);
                    openSet.push(nextState);
                }
            }
        }

        cout << "Nodes explored with A*: " << nodesExplored << "\n";
        return "NO SOLUTION";
    }

    void playback(const string& solutionPath) {
        resetLevel();
        int px = initialPlayerX;
        int py = initialPlayerY;

        cout << "\n========== PLAYBACK START (A* SOLUTION) ==========\n\n";
        cout << "Step 0 (Initial State)\n";
        cout << "Target Path: " << solutionPath << "\n";
        render();
        cout << flush;

        Sleep(1000);

        int step = 1;
        for (char move : solutionPath) {
            simulateMove(grid, px, py, move);

            cout << "------------------------------------\n";
            cout << "Step " << step++ << " / " << solutionPath.length()
                 << " | Played Move: " << move << "\n";
            render();
            cout << flush;

            Sleep(800);
        }

        cout << "========== LEVEL CLEAR! ==========\n";
        cout << "Total Winning Path: " << solutionPath << "\n";
        cout << "Path Length: " << solutionPath.length() << " moves\n\n";
    }
};

int main() {
    SokobanAStar game;
    string highlevelString = "13: ###  ####    # ####  #    # $     #    # ###   #   ##   ### #####      @    ##   ## # ## ##.####      ####  ########";
    string midlevelString = "12:      ####   #######  #  #        #  #       .# #@  ####  # #         # # # # #   # #       $ # # ####    #  ####       ";

    if (!game.loadLevel(highlevelString)) {
        cout << "Failed to load level.\n";
        return 1;
    }

    cout << "Calculating solution using A* algorithm... (This may be faster than BFS)\n";
    string solution = game.solveAStar();

    if (solution == "NO SOLUTION") {
        cout << "Could not find a valid path to win.\n";
    } else {
        game.playback(solution);
    }

    return 0;
}
