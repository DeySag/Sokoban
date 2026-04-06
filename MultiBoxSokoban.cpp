#include <iostream>
#include <bits/stdc++.h>

using namespace std;

// Parses the flat string into a 2D matrix
vector<string> parseGrid(const string& flat_string) {
    size_t colon_pos = flat_string.find(':');
    if (colon_pos == string::npos) {
        cout << "Invalid input !\n";
        return {"-1"};
    }
    
    int cols = stoi(flat_string.substr(0, colon_pos));
    string flat = flat_string.substr(colon_pos + 1);
    
    cout << cols << "\n";

    if (flat.length() % cols != 0) {
        cout << "Invalid input !\n";
        return {"-1"};
    }

    vector<string> matrix;
    for (size_t i = 0; i < flat.length(); i += cols) {
        matrix.push_back(flat.substr(i, cols));
    }
    return matrix;
}

// Prints the grid with space separation
void printGrid(const vector<string>& matrix) {
    for (const string& row : matrix) {
        for (size_t i = 0; i < row.length(); ++i) {
            cout << row[i] << (i == row.length() - 1 ? "" : " ");
        }
        cout << "\n";
    }
}

// Modifies the matrix and updates the current player coordinates and target count
void moveChange(vector<string>& matrix, char move, int& curr_i, int& curr_j, int& count_tar) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    
    int di = 0, dj = 0;
    if (move == 'w') di = -1;
    else if (move == 's') di = 1;
    else if (move == 'a') dj = -1;
    else if (move == 'd') dj = 1;

    int i = curr_i + di;
    int j = curr_j + dj;

    // Check boundary or wall collision for player
    if (i < 0 || i >= rows || j < 0 || j >= cols || matrix[i][j] == '#') {
        cout << "Invalid Move !\n";
        return;
    }

    // Walking on empty space or goal
    if (matrix[i][j] == ' ' || matrix[i][j] == '.') {
        matrix[curr_i][curr_j] = (matrix[curr_i][curr_j] == '+') ? '.' : ' ';
        matrix[i][j] = (matrix[i][j] == ' ') ? '@' : '+';
        curr_i = i;
        curr_j = j;
    } 
    // Pushing a box
    else if (matrix[i][j] == '$' || matrix[i][j] == '*') {
        int i2 = i + di;
        int j2 = j + dj;

        // Check boundary or wall/box collision for the pushed box
        if (i2 < 0 || i2 >= rows || j2 < 0 || j2 >= cols || 
            matrix[i2][j2] == '#' || matrix[i2][j2] == '*' || matrix[i2][j2] == '$') {
            cout << "Invalid Move !\n";
            return;
        }

        // Pushing box onto a goal
        if (matrix[i2][j2] == '.') {
            if (matrix[i][j] == '$') count_tar--; 
            matrix[i2][j2] = '*';
            
            matrix[i][j] = (matrix[i][j] == '*') ? '+' : '@';
        } 
        // Pushing box onto an empty floor
        else {
            matrix[i2][j2] = '$';
            
            if (matrix[i][j] == '*') {
                matrix[i][j] = '+';
                count_tar++; 
            } else {
                matrix[i][j] = '@';
            }
        }
        
        matrix[curr_i][curr_j] = (matrix[curr_i][curr_j] == '+') ? '.' : ' ';
        curr_i = i;
        curr_j = j;
    }
}

int main() {
    string flat_string;
    cout << "Enter Formatted String : <{cols}:{elements}> : ";
    getline(cin, flat_string);

    vector<string> matrix = parseGrid(flat_string);
    if (!matrix.empty() && matrix[0] == "-1") {
        return 0; // Invalid input triggered
    }

    cout << "Board : \n\n";
    printGrid(matrix);

    int count_tar = 0;
    int curr_i = -1, curr_j = -1;

    // Find initial player position and count un-goaled boxes
    for (int i = 0; i < matrix.size(); ++i) {
        for (int j = 0; j < matrix[0].size(); ++j) {
            if (matrix[i][j] == '+' || matrix[i][j] == '@') {
                curr_i = i;
                curr_j = j;
            }
            if (matrix[i][j] == '$') {
                count_tar++;
            }
        }
    }

    string move_input;
    while (true) {
        cout << "\n";
        
        if (count_tar == 0) {
            cout << "Game Won!\n";
            break;
        }

        cout << "Enter Key : ";
        getline(cin, move_input);

        if (move_input.empty()) {
            cout << "Invalid Move !\n";
            printGrid(matrix);
            continue;
        }

        char move = tolower(move_input[0]);

        if (move == 'q') {
            cout << "Exiting Game !\n";
            break;
        } else if (move != 'w' && move != 'a' && move != 's' && move != 'd') {
            cout << "Move Invalid !\n";
            printGrid(matrix);
            continue;
        }

        moveChange(matrix, move, curr_i, curr_j, count_tar);
        
        cout << "\n";
        printGrid(matrix);

        if (count_tar == 0) {
            cout << "Game Won !!\n";
            printGrid(matrix); 
            break;
        }
    }

    return 0;
}
