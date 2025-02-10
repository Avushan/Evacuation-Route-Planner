#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <climits>
#include <unistd.h>    // For sleep()
#include <cstdlib>     // For system()
#include <algorithm>   // For reverse()

using namespace std;


int directions[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };

const char PERSON = 'P', FIRE = 'F', EXIT = 'E', WALL = '#', OPEN = '.', PATH = 'O';

// The grid (ASCII map)
vector<vector<char>> grid = {
    {'S', '.', '.', '#', 'F'},
    {'.', '#', '.', '#', '.'},
    {'.', '.', '.', '.', '#'},
    {'#', '.', '#', '.', '.'},
    {'.', '.', '.', '#', 'E'}
};

int rows = grid.size();
int cols = grid[0].size();
vector<pair<int, int>> finalPath;  // Store final path

void clearScreen() {
#ifdef _WIN32
    system("CLS");
#else
    system("clear");
#endif
}

//Print the grid
void printGrid() {
    for (auto &row : grid) {
        for (char cell : row) {
            cout << cell << " ";
        }
        cout << endl;
    }
    cout << endl;
}

// Check if a cell is safe to step into
bool isSafeCell(int r, int c) {
    if (grid[r][c] == FIRE) return false;
    for (int i = 0; i < 4; i++) {
        int nr = r + directions[i][0], nc = c + directions[i][1];
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && grid[nr][nc] == FIRE) {
            return false;  // Adjacent to fire
        }
    }
    return true;
}

// Use BFS to find the safest path
vector<pair<int, int>> computeSafePath(pair<int, int> start) {
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    queue<pair<pair<int, int>, int>> q; // ((row, col), steps)
    map<pair<int, int>, pair<int, int>> prev;

    q.push({start, 0});
    visited[start.first][start.second] = true;

    pair<int, int> exitPos = {-1, -1};
    bool found = false;

    while (!q.empty()) {
        auto front = q.front();
        q.pop();
        pair<int, int> pos = front.first;
        int r = pos.first, c = pos.second;

        // If we reached the exit
        if (grid[r][c] == EXIT) {
            exitPos = {r, c};
            found = true;
            break;
        }

        // Explore 4 possible directions
        for (int i = 0; i < 4; i++) {
            int nr = r + directions[i][0], nc = c + directions[i][1];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                char cell = grid[nr][nc];
                if (cell == PERSON) cell = OPEN;
                if (cell != WALL && cell != FIRE && !visited[nr][nc] && isSafeCell(nr, nc)) {
                    visited[nr][nc] = true;
                    prev[{nr, nc}] = {r, c};
                    q.push({{nr, nc}, front.second + 1});
                }
            }
        }
    }

    vector<pair<int, int>> path;
    if (!found) return path; // No safe path

    // Reconstruct path
    pair<int, int> cur = exitPos;
    while (cur != start) {
        path.push_back(cur);
        cur = prev[cur];
    }
    reverse(path.begin(), path.end());
    return path;
}

// Spread fire to adjacent open cells
void spreadFire() {
    vector<pair<int, int>> newFires;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == FIRE) {
                for (int i = 0; i < 4; i++) {
                    int nr = r + directions[i][0], nc = c + directions[i][1];
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (grid[nr][nc] == OPEN || grid[nr][nc] == PERSON)
                            newFires.push_back({nr, nc});
                    }
                }
            }
        }
    }
    for (auto &p : newFires) {
        grid[p.first][p.second] = FIRE;
    }
}

int main() {
    // Find the starting position ('S') and replace it with 'P'
    pair<int, int> personPos;
    for (int r = 0; r < rows; r++){
        for (int c = 0; c < cols; c++){
            if (grid[r][c] == 'S') {
                personPos = {r, c};
                grid[r][c] = PERSON;  // Mark person position
                break;
            }
        }
    }

    // Simulation loop
    while (true) {
        clearScreen();
        printGrid();

        // If the person reaches the exit
        if (grid[personPos.first][personPos.second] == EXIT) {
            cout << "Person reached the exit safely! 🎉" << endl;
            finalPath = computeSafePath({personPos.first, personPos.second});
            break;
        }

        // Compute a safe path
        vector<pair<int, int>> path = computeSafePath(personPos);
        if (path.empty()) {
            cout << "No safe path found! The person is trapped." << endl;
            break;
        }

        // Move one step along the path
        pair<int, int> nextPos = path[0];
        grid[personPos.first][personPos.second] = PATH; // Mark old position with 'O'
        if (grid[nextPos.first][nextPos.second] != EXIT) {
            grid[nextPos.first][nextPos.second] = PERSON;
        }
        personPos = nextPos;

        // Let the fire spread
        spreadFire();

        // Pause for a short duration
        usleep(100000); // 0.1 seconds
    }

    // Show final safest path
    if (!finalPath.empty()) {
        usleep(100000); // 0.1 seconds
        clearScreen();
        for (auto &p : finalPath) {
            if (grid[p.first][p.second] != EXIT) {
                grid[p.first][p.second] = PATH;
            }
        }
        printGrid();
        cout << "The safest path is marked with 'O'." << endl;
    }

    return 0;
}
