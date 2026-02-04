#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <raylib.h>


// CONSTS
#define ONE_UNIT 70
#define ONE_LINE_UNIT 4
#define MAX_FPS 60
// STATES
#define STATE_DEAD 0
#define STATE_ALIVE 1
#define STATE_WON 2
// CUZS
#define CUZ_INVALID_PLACE 'i'
#define CUZ_LIMIT_REACHED 'l'
// TYPES
#define TYPE_HUNTER 'h'
#define TYPE_WALL 'w'
#define TYPE_WALL_H '_'
#define TYPE_WALL_V '|'
#define TYPE_FREE_WAY 'f'
#define TYPE_RUNNER 'r'
#define TYPE_GOAL 'g'
#define TYPE_PRIZE 'p' 
// COLORS
#define COLOR_FREE_WAY BLACK
#define COLOR_NOT_WALL WHITE
#define COLOR_WALL MAGENTA
#define COLOR_TEMP_WALL VIOLET
#define COLOR_RUNNER BLUE
const Color COLOR_RUNNER_ACTIVE = {0, 241, 241, 255};
#define COLOR_HUNTER RED
#define COLOR_GOAL YELLOW
#define COLOR_WON GREEN
#define COLOR_LOST RED
#define COLOR_WARN MAGENTA


typedef struct 
{
    char type;
} Entity;
typedef struct
{
    int x;
    int y;
} Point;
typedef struct
{
    int i;
    int j;
    int state;
} Player;
typedef struct
{
    int i;
    int j;
} Enemy;
typedef struct
{
    int i;
    int j;
    char type;
    int iter;
} TempWall;
typedef struct
{
    int i;
    int j;
    int active;
} Prize;


int max(int a, int b)
{
    int gonde = (a>=b) ? a : b;
    return gonde;
}
void putInRange(int a, int b, int* n)
{
    if (*n < a) *n = a;
    else if (*n > b) *n = b;
}
int isTempWallAt(int i, int j, char type, int maxTempWallCount, TempWall tempWalls[maxTempWallCount])
{
    for (int k = 0; k!=maxTempWallCount; k++)
        if (tempWalls[k].i == i && tempWalls[k].j == j && tempWalls[k].type == type)
            return 1;
    return 0;
}
int min(int a, int b)
{
    int kuch = (a<=b) ? a : b;
    return kuch;
}

float eucDist(float x1, float y1, float x2, float y2)
{
    return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
}

int boundCheck(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int playerI, int playerJ, char where, int maxTempWallCount, TempWall tempwalls[maxTempWallCount])
{
    switch (where)
    {
        case 'W':
            if (playerI==0) return 0;
            if (hWalls[playerI-1][playerJ]) return 0;
            if (gameboard[playerI-1][playerJ].type == TYPE_FREE_WAY || gameboard[playerI-1][playerJ].type == TYPE_GOAL)
            {
                for (int i = 0; i!=maxTempWallCount; i++)
                    if (tempwalls[i].i==playerI-1 && tempwalls[i].j==playerJ && tempwalls[i].type == TYPE_WALL_H)
                        return 0;
                return 1;
            }
            return 0;
            break;
        case 'A':
            if (playerJ==0) return 0;
            if (vWalls[playerI][playerJ-1]) return 0;
            if (gameboard[playerI][playerJ-1].type == TYPE_FREE_WAY || gameboard[playerI][playerJ-1].type == TYPE_GOAL)
            {
                for (int i = 0; i!=maxTempWallCount; i++)
                    if (tempwalls[i].i==playerI && tempwalls[i].j==playerJ-1 && tempwalls[i].type == TYPE_WALL_V)
                        return 0;
                return 1;
            }
            return 0;
            break;
        case 'S':
            if (playerI==rows-1) return 0;
            if (hWalls[playerI][playerJ]) return 0;
            if (gameboard[playerI+1][playerJ].type == TYPE_FREE_WAY || gameboard[playerI+1][playerJ].type == TYPE_GOAL)
            {
                for (int i = 0; i!=maxTempWallCount; i++)
                    if (tempwalls[i].i==playerI && tempwalls[i].j==playerJ && tempwalls[i].type == TYPE_WALL_H)
                        return 0;
                return 1;
            }
            return 0;
            break;
        case 'D':
            if (playerJ==cols-1) return 0;
            if (vWalls[playerI][playerJ]) return 0;
            if (gameboard[playerI][playerJ+1].type == TYPE_FREE_WAY || gameboard[playerI][playerJ+1].type == TYPE_GOAL)
            {
                for (int i = 0; i!=maxTempWallCount; i++)
                    if (tempwalls[i].i==playerI && tempwalls[i].j==playerJ && tempwalls[i].type == TYPE_WALL_V)
                        return 0;
                return 1;
            }
            return 0;
            break;
    }
    return 0;
}

int didWin(int runnerCount, Player players[runnerCount])
{
    int winnerCount = 0;
    for (int i = 0; i!=runnerCount; i++)
        if (players[i].state == STATE_WON)
            winnerCount++;

    return (winnerCount >= ((float)runnerCount)/3);
}
int didLose(int rows, int cols, Entity gameboard[rows][cols])
{
    int v_didLose = 1;
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            if (gameboard[i][j].type == TYPE_RUNNER)
                v_didLose = 0;
    return v_didLose;
}

Color rainbowColor(float t)
{
    int r = (127 + 127*sin(t));
    putInRange(0, 255, &r);
    int g = (127 + 127*sin(t + 2.0)); // 120deg = 2pi/3 ~~ 2 rad 
    putInRange(0, 255, &g);
    int b = (127 + 127*sin(t + 4.0)); // 240deg = 4pi/3 ~~ 4 rad
    putInRange(0, 255, &b);
    return (Color){r, g, b, 255};
}

int isTempWallBlockingCell(int i, int j, int maxTempWallCount, TempWall tempWalls[maxTempWallCount])
{
    for (int k = 0; k != maxTempWallCount; k++)
    {
        if (tempWalls[k].i == -1) continue;

        if (tempWalls[k].type == TYPE_WALL_H)
        {
            if ((tempWalls[k].i == i && tempWalls[k].j == j) || (tempWalls[k].i + 1 == i && tempWalls[k].j == j))
                return 1;
        }
        else if (tempWalls[k].type == TYPE_WALL_V)
        {
            if ((tempWalls[k].i == i && tempWalls[k].j == j) || (tempWalls[k].i == i && tempWalls[k].j + 1 == j))
                return 1;
        }
    }
    return 0;
}

int canSpawnPrizeAt(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int maxTempWallCount, TempWall tempWalls[maxTempWallCount], int i, int j)
{
    if (i < 0 || i >= rows || j < 0 || j >= cols) return 0;
    if (gameboard[i][j].type != TYPE_FREE_WAY) return 0;
    if (isTempWallBlockingCell(i, j, maxTempWallCount, tempWalls)) return 0;

    return 1;
}

void earthquake(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int runnerCount, Player players[runnerCount], int maxTempWallCount, TempWall tempWalls[maxTempWallCount], int goalY, int goalX)
{
    int hunterCount = 0;
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            if (gameboard[i][j].type == TYPE_HUNTER) hunterCount++;

    Enemy hunters[hunterCount];
    int hc = 0;
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            if (gameboard[i][j].type == TYPE_HUNTER)
                hunters[hc++] = (Enemy){i, j};

    int occupied[rows][cols];
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            occupied[i][j] = (gameboard[i][j].type != TYPE_FREE_WAY);

    int di[4] = {-1, 1, 0, 0};
    int dj[4] = {0, 0, -1, 1};

    // runners
    for (int p = 0; p != runnerCount; p++)
    {
        if (players[p].state != STATE_ALIVE) continue;

        int ci = players[p].i;
        int cj = players[p].j;

        int tries = 4;
        for (; tries != 0; tries--)
        {
            int d = rand() % 4;
            int ni = ci + di[d];
            int nj = cj + dj[d];

            if (ni == goalY && nj == goalX) continue;

            if (!canSpawnPrizeAt(rows, cols, gameboard, hWalls, vWalls, maxTempWallCount, tempWalls, ni, nj))
                continue;

            if (occupied[ni][nj]) continue;

            if (!(ci == goalY && cj == goalX))
                gameboard[ci][cj].type = TYPE_FREE_WAY;

            // move runner
            players[p].i = ni;
            players[p].j = nj;
            gameboard[ni][nj].type = TYPE_RUNNER;

            occupied[ci][cj] = 0;
            occupied[ni][nj] = 1;
            break;
        }
    }

    // hunters
    for (int h = 0; h != hunterCount; h++)
    {
        int ci = hunters[h].i;
        int cj = hunters[h].j;

        int tries = 4;
        for (; tries != 0; tries--)
        {
            int d = rand() % 4;
            int ni = ci + di[d];
            int nj = cj + dj[d];

            if (ni == goalY && nj == goalX) continue;

            if (!canSpawnPrizeAt(rows, cols, gameboard, hWalls, vWalls, maxTempWallCount, tempWalls, ni, nj))
                continue;

            if (occupied[ni][nj]) continue;

            if (!(ci == goalY && cj == goalX))
                gameboard[ci][cj].type = TYPE_FREE_WAY;

            gameboard[ni][nj].type = TYPE_HUNTER;
            hunters[h].i = ni;
            hunters[h].j = nj;


            occupied[ci][cj] = 0;
            occupied[ni][nj] = 1;
            break;
        }
    }

    // just in case :<
    gameboard[goalY][goalX].type = TYPE_GOAL;
}


void moveHuntersX(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int runnerCount, Player players[runnerCount], int maxTempWallCount, TempWall tempWalls[maxTempWallCount])
{
    int hunterCount = 0;
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            if (gameboard[i][j].type == TYPE_HUNTER) hunterCount++;

    Enemy enemies[hunterCount];
    int enemyCursor = 0;
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            if (gameboard[i][j].type == TYPE_HUNTER) enemies[enemyCursor++] = (Enemy){i, j};

    for (int h = 0; h!=hunterCount; h++)
    {
        int iMe = enemies[h].i;
        int jMe = enemies[h].j;

        int targetI = -1; int targetJ = -1;
        float minExDist = 1000000.0f; 

        for (int p = 0; p!=runnerCount; p++)
        {
            if (players[p].state == STATE_ALIVE)
            {
                float d = eucDist(jMe, iMe, players[p].j, players[p].i);
                if (d < minExDist)
                {
                    minExDist = d;
                    targetI = players[p].i;
                    targetJ = players[p].j;
                }
            }
        }
        if (targetI == -1) continue;

        for (int moveCount = 0; moveCount < 2; moveCount++)
        {
            int oldI = iMe;
            int oldJ = jMe;

            if (targetJ > jMe && !vWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe+1].type == TYPE_FREE_WAY || gameboard[iMe][jMe+1].type == TYPE_RUNNER))
                jMe++;
            else if (targetJ < jMe && !vWalls[iMe][jMe-1] && !isTempWallAt(iMe, jMe-1, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe-1].type == TYPE_FREE_WAY || gameboard[iMe][jMe-1].type == TYPE_RUNNER))
                jMe--;
            else if (targetI > iMe && !hWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe+1][jMe].type == TYPE_FREE_WAY || gameboard[iMe+1][jMe].type == TYPE_RUNNER))
                iMe++;
            else if (targetI < iMe && !hWalls[iMe-1][jMe] && !isTempWallAt(iMe-1, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe-1][jMe].type == TYPE_FREE_WAY || gameboard[iMe-1][jMe].type == TYPE_RUNNER))
                iMe--;

            if (oldI != iMe || oldJ != jMe)
            {
                gameboard[oldI][oldJ].type = TYPE_FREE_WAY;
                gameboard[iMe][jMe].type = TYPE_HUNTER;
            }
            else
                break;
        }
    }
}

void moveHunters(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int iPlayer, int jPlayer, int maxTempWallCount, TempWall tempWalls[maxTempWallCount])
{
    int hunterCount = 0;
    for (int i = 0; i != rows; i++)
        for (int j = 0; j != cols; j++)
            if (gameboard[i][j].type == TYPE_HUNTER)
                hunterCount++;
    int iMe;
    int jMe;

    Enemy enemies[hunterCount];
    int enemyCursor = 0;
    // find self
    for (int i = 0; i != rows; i++)
    {
        for (int j = 0; j != cols; j++)
        {
            if (gameboard[i][j].type == TYPE_HUNTER)
            {
                Enemy newE = {i, j};
                enemies[enemyCursor++] = newE;
            }
        }
    }
    // END find self
    for (int i = 0; i != hunterCount; i++)
    {
        iMe = enemies[i].i;
        jMe = enemies[i].j;
        for (int moveCount = 0; moveCount < 2; moveCount++)
        {
            // if must go right
            if (jPlayer > jMe)
            {
                // should move directly
                if (!vWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe + 1].type == TYPE_FREE_WAY || gameboard[iMe][jMe + 1].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe][jMe + 1].type = TYPE_HUNTER;
                    jMe++;
                    continue;
                }
                // shouldnt move
                if (iMe == iPlayer)
                    continue;
                // should move indirectly
                if (iMe > iPlayer)
                {
                    if (iMe != 0 && !hWalls[iMe - 1][jMe] && !isTempWallAt(iMe - 1, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe - 1][jMe].type == TYPE_FREE_WAY || gameboard[iMe - 1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe - 1][jMe].type = TYPE_HUNTER;
                        iMe--;
                        continue;
                    }
                }
                else
                {
                    if (iMe != rows - 1 && !hWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe + 1][jMe].type == TYPE_FREE_WAY || gameboard[iMe + 1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe + 1][jMe].type = TYPE_HUNTER;
                        iMe++;
                        continue;
                    }
                }
            }
            // if must go left
            if (jMe > jPlayer)
            {
                // should move directly
                if (!vWalls[iMe][jMe - 1] && !isTempWallAt(iMe, jMe - 1, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe - 1].type == TYPE_FREE_WAY || gameboard[iMe][jMe - 1].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe][jMe - 1].type = TYPE_HUNTER;
                    jMe--;
                    continue;
                }
                // shouldnt move
                if (iMe == iPlayer)
                    continue;
                // should move indirectly
                if (iMe > iPlayer)
                {
                    if (iMe != 0 && !hWalls[iMe - 1][jMe] && !isTempWallAt(iMe - 1, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe - 1][jMe].type == TYPE_FREE_WAY || gameboard[iMe - 1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe - 1][jMe].type = TYPE_HUNTER;
                        iMe--;
                        continue;
                    }
                }
                else
                {
                    if (iMe != rows - 1 && !hWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe + 1][jMe].type == TYPE_FREE_WAY || gameboard[iMe + 1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe + 1][jMe].type = TYPE_HUNTER;
                        iMe++;
                        continue;
                    }
                }
            }
            // END if must go left
            // if must go up
            if (iMe > iPlayer)
            {
                // should move directly
                if (!hWalls[iMe - 1][jMe] && !isTempWallAt(iMe - 1, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe - 1][jMe].type == TYPE_FREE_WAY || gameboard[iMe - 1][jMe].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe - 1][jMe].type = TYPE_HUNTER;
                    iMe--;
                    continue;
                }
                // shouldnt move
                if (jMe == jPlayer)
                    continue;
                // should move indirectly
                if (jMe > jPlayer)
                {
                    if (jMe != 0 && !vWalls[iMe][jMe - 1] && !isTempWallAt(iMe, jMe - 1, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe - 1].type == TYPE_FREE_WAY || gameboard[iMe][jMe - 1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe - 1].type = TYPE_HUNTER;
                        jMe--;
                        continue;
                    }
                }
                else
                {
                    if (jMe != cols - 1 && !vWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe + 1].type == TYPE_FREE_WAY || gameboard[iMe][jMe + 1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe + 1].type = TYPE_HUNTER;
                        jMe++;
                        continue;
                    }
                }
            }
            // END if must go up
            // if must go down
            if (iMe < iPlayer)
            {
                // should move directly
                if (!hWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_H, maxTempWallCount, tempWalls) && (gameboard[iMe + 1][jMe].type == TYPE_FREE_WAY || gameboard[iMe + 1][jMe].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe + 1][jMe].type = TYPE_HUNTER;
                    iMe++;
                    continue;
                }
                // shouldnt move
                if (jMe == jPlayer)
                    continue;
                // should move indirectly
                if (jMe > jPlayer)
                {
                    if (jMe != 0 && !vWalls[iMe][jMe - 1] && !isTempWallAt(iMe, jMe - 1, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe - 1].type == TYPE_FREE_WAY || gameboard[iMe][jMe - 1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe - 1].type = TYPE_HUNTER;
                        jMe--;
                        continue;
                    }
                }
                else
                {
                    if (jMe != cols - 1 && !vWalls[iMe][jMe] && !isTempWallAt(iMe, jMe, TYPE_WALL_V, maxTempWallCount, tempWalls) && (gameboard[iMe][jMe + 1].type == TYPE_FREE_WAY || gameboard[iMe][jMe + 1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe + 1].type = TYPE_HUNTER;
                        jMe++;
                        continue;
                    }
                }
            }
        }
    }

    return;
}

void showText(char* msg, Color color)
{
    int fontSize = 20;
    
    int textWidth = MeasureText(msg, fontSize);
    int posX = (GetScreenWidth()-textWidth)/2;
    int posY = (GetScreenHeight()-fontSize)/2;

    BeginDrawing();
        ClearBackground(BLACK);
        
        DrawRectangle(posX-10, posY-10, textWidth+20, fontSize+20, Fade(BLACK, 0.8f));
        
        DrawText(msg, posX, posY, fontSize, color);
    EndDrawing();

    sleep(2);
}

int logic(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int* isPlayerTurn, int maxTempWallCount, TempWall tempWalls[maxTempWallCount], int runnerCount, Player players[runnerCount], char diff)
{
    // returns 0 if must continuue
    // returns -1 if lost
    // returns 1 if won

    // find player
    int iPlayer = -1;
    int jPlayer = -1;
    for (int i = 0; i!=rows && iPlayer==-1; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (gameboard[i][j].type == TYPE_RUNNER)
            {
                iPlayer = i;
                jPlayer = j;
            }
        }
    }
    // hunters should go
    if (!(*isPlayerTurn))
    {
        if (diff=='e')
            moveHunters(rows, cols, gameboard, hWalls, vWalls, iPlayer, jPlayer, maxTempWallCount, tempWalls);
        else if (diff=='h')
            moveHuntersX(rows, cols, gameboard, hWalls, vWalls, runnerCount, players, maxTempWallCount, tempWalls);
        
        // temp wall iter++
        for (int i = 0; i!=maxTempWallCount; i++)
            if (tempWalls[i].iter >= 2)
            {
                tempWalls[i].i=-1;
                tempWalls[i].j=-1;
                tempWalls[i].iter=0;
            }
        for (int i = 0; i!=maxTempWallCount; i++)
            if (tempWalls[i].i != -1)
                tempWalls[i].iter++;

            
        *isPlayerTurn = 1;
    }
    // un-alive deleted players
    for (int i = 0; i!=runnerCount; i++)
    {
        if (players[i].state == STATE_ALIVE)
        {
            if (gameboard[players[i].i][players[i].j].type != TYPE_RUNNER)
            {
                if (gameboard[players[i].i][players[i].j].type == TYPE_GOAL)
                    players[i].state = STATE_WON;
                else
                    players[i].state = STATE_DEAD;
            }
        }
    }

    if (didWin(runnerCount, players))
    {
        showText("YOU WON", COLOR_WON);
        return 1;
    }
    if (didLose(rows, cols, gameboard))
    {
        showText("YOU LOST BRO, GET OUTTA HERE", COLOR_LOST);
        return -1;
    }
    
    return 0;
}

void warnWall(char reason)
{
    char msg[50];
    if (reason==CUZ_LIMIT_REACHED)
        strcpy(msg, "MAXIMUM LIMIT REACHED BOI.");
    else if (reason==CUZ_INVALID_PLACE)
        strcpy(msg, "PLACE IT SOMEWHERE ELSE.");
    int fontSize = 20;
    
    int textWidth = MeasureText(msg, fontSize);
    int posX = (GetScreenWidth()-textWidth)/2;
    int posY = (GetScreenHeight()-fontSize)/2;

    BeginDrawing();
        ClearBackground(BLACK);
        
        DrawRectangle(posX-10, posY-10, textWidth+20, fontSize+20, Fade(BLACK, 0.8f));
        
        DrawText(msg, posX, posY, fontSize, COLOR_WARN);
    EndDrawing();

    sleep(1);
}

void warnMove()
{
    char* msg = "MOVE YOUR AHH SOMEWHERE ELSE.";
    int fontSize = 20;
    
    int textWidth = MeasureText(msg, fontSize);
    int posX = (GetScreenWidth()-textWidth)/2;
    int posY = (GetScreenHeight()-fontSize)/2;

    BeginDrawing();
        ClearBackground(BLACK);
        
        DrawRectangle(posX-10, posY-10, textWidth+20, fontSize+20, Fade(BLACK, 0.8f));
        
        DrawText(msg, posX, posY, fontSize, COLOR_WARN);
    EndDrawing();

    sleep(1);
}

void drawMap(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int maxTempWallCount, TempWall tempWalls[maxTempWallCount], int runnerCount, Player players[runnerCount], int activeRunner, Prize prize)
{
    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            Color cellColor = COLOR_FREE_WAY;
            if (gameboard[i][j].type == TYPE_HUNTER) cellColor = COLOR_HUNTER;
            else if (gameboard[i][j].type == TYPE_RUNNER)
            {
                cellColor = COLOR_RUNNER;
                if (activeRunner >= 0 && activeRunner < runnerCount && players[activeRunner].state == STATE_ALIVE && players[activeRunner].i == i && players[activeRunner].j == j)
                    cellColor = COLOR_RUNNER_ACTIVE;

            }
            else if (gameboard[i][j].type == TYPE_GOAL) cellColor = COLOR_GOAL;
            
            if (gameboard[i][j].type != TYPE_GOAL)
                DrawCircleLines(j*ONE_UNIT + ONE_UNIT/2, i*ONE_UNIT + ONE_UNIT/2, ONE_UNIT/2.5, cellColor);
            else
                DrawCircle(j*ONE_UNIT + ONE_UNIT/2, i*ONE_UNIT + ONE_UNIT/2, ONE_UNIT/2, cellColor);

            if (prize.active && prize.i == i && prize.j == j)
            {
                Color pColor = rainbowColor(GetTime()*4.0);
                DrawCircle(j*ONE_UNIT + ONE_UNIT/2, i*ONE_UNIT + ONE_UNIT/2, ONE_UNIT/3, pColor);
            }

            DrawRectangleLines(j*ONE_UNIT, i*ONE_UNIT, ONE_UNIT, ONE_UNIT, COLOR_NOT_WALL);
        }
    }
    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (hWalls[i][j])
                DrawRectangle(j*ONE_UNIT, (i+1)*ONE_UNIT-(ONE_LINE_UNIT/2), ONE_UNIT, ONE_LINE_UNIT, COLOR_WALL);
            if (vWalls[i][j])
                DrawRectangle((j+1)*ONE_UNIT-(ONE_LINE_UNIT/2), i*ONE_UNIT, ONE_LINE_UNIT, ONE_UNIT, COLOR_WALL);
        }        
    }
    // temp walls
    for (int i = 0; i!=maxTempWallCount; i++)
    {
        if (tempWalls[i].i != -1)
        {
            if (tempWalls[i].type == TYPE_WALL_H)
                DrawRectangle(tempWalls[i].j*ONE_UNIT, (tempWalls[i].i+1)*ONE_UNIT-(ONE_LINE_UNIT/2), ONE_UNIT, ONE_LINE_UNIT, COLOR_TEMP_WALL);
            else if (tempWalls[i].type == TYPE_WALL_V)
                DrawRectangle((tempWalls[i].j+1)*ONE_UNIT-(ONE_LINE_UNIT/2), tempWalls[i].i*ONE_UNIT, ONE_LINE_UNIT, ONE_UNIT, COLOR_TEMP_WALL);
        }

    }
}

float random0n1(int mult)
{
    double randomNumber = (double)rand();
    while (randomNumber>1) randomNumber /= 10;
    // 0.783845621
    randomNumber = (double)((int)(randomNumber*10)/10.0);
    return (float)(randomNumber*mult);
}

int isOk(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int maxTempWallCount, TempWall tempWalls[maxTempWallCount])
{
    int visited[rows][cols];
    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
            visited[i][j] = 0;
    }

    int startI = -1, startJ = -1;
    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (gameboard[i][j].type == TYPE_RUNNER)
            {
                startI = i;
                startJ = j;
                break;
            }
        }
        if (startI != -1) break;
    }
    if (startI == -1) return 1;

    int stackI[rows*cols];
    int stackJ[rows*cols];
    int top = 0;
    stackI[top] = startI;
    stackJ[top] = startJ;
    top++;
    visited[startI][startJ] = 1;

    while (top>0)
    {
        top--;
        int ci = stackI[top];
        int cj = stackJ[top];

        int di[4] = {-1, 1, 0, 0};
        int dj[4] = {0, 0, -1, 1};
        char dir[4] = {'W', 'S', 'A', 'D'};

        for (int d = 0; d < 4; d++)
        {
            int ni = ci + di[d];
            int nj = cj + dj[d];

            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols)
            {
                if (!visited[ni][nj])
                {
                    int canMove = boundCheck(rows, cols, gameboard, hWalls, vWalls, ci, cj, dir[d], maxTempWallCount, tempWalls);
                    
                    if (!canMove && !hWalls[ci][cj] && !vWalls[ci][cj])
                    {
                        if (gameboard[ni][nj].type == TYPE_GOAL)
                        {
                            if (dir[d] == 'W' && ci > 0 && !hWalls[ci-1][cj]) canMove = 1;
                            if (dir[d] == 'S' && ci < rows-1 && !hWalls[ci][cj]) canMove = 1;
                            if (dir[d] == 'A' && cj > 0 && !vWalls[ci][cj-1]) canMove = 1;
                            if (dir[d] == 'D' && cj < cols-1 && !vWalls[ci][cj]) canMove = 1;
                        }
                    }

                    if (canMove)
                    {
                        visited[ni][nj] = 1;
                        stackI[top] = ni;
                        stackJ[top] = nj;
                        top++;
                    }
                }
            }
        }
    }

    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (gameboard[i][j].type == TYPE_GOAL && !visited[i][j])
            return 0;
        }
    }
    return 1;
}
void spawnPrize(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int maxTempWallCount, TempWall tempWalls[maxTempWallCount], Prize* prize)
{
    prize->active = 0;
    prize->i = -1;
    prize->j = -1;

    // NOTE 10% CHANCE PER CELL
    int CHANCE = 10;
    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (!canSpawnPrizeAt(rows, cols, gameboard, hWalls, vWalls, maxTempWallCount, tempWalls, i, j))
                continue;
            if ((rand() % 100) < CHANCE)
            {
                prize->active = 1;
                prize->i = i;
                prize->j = j;
                return;
            }
        }
    }
}




int main()
{
    srand(time(NULL));

    // get inputs
    int w; int h;
    while (1)
    {
        printf("ENTER n AND m RESPECTIVELY (both in {6, 7, 8, 9, 10}): ");
        scanf("%d %d", &w, &h);
        printf("\n");
        if (6<=w && w<=10 && 6<=h && h<=10) break;
    }
    /* NOTE 1.0 */
    const int ROWS = h; const int COLS = w;
    Entity GAMEBOARD[ROWS][COLS]; // hunter or runner or goal
    int hWalls[ROWS][COLS]; // 0 or 1
    int vWalls[ROWS][COLS]; // 0 or 1
    int maxTempWalls = max(min(ROWS, COLS)/3, 1);
    int tempWallsCap = maxTempWalls + 20;
    TempWall* tempWalls = malloc(sizeof(TempWall) * tempWallsCap);

    for (int i = 0; i!=tempWallsCap; i++)
        tempWalls[i] = (TempWall){-1, -1, TYPE_WALL_H, -1};
    Prize prize = (Prize){-1, -1, 0};
    // init board to TYPE_FREE_WAY
    for (int i = 0; i!=ROWS; i++)
    {
        for (int j = 0; j!=COLS; j++)
        {
            Entity newEntity = {TYPE_FREE_WAY};
            GAMEBOARD[i][j] = newEntity;
            hWalls[i][j] = 0;
            vWalls[i][j] = 0;
        }
    }
    // TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP
    // haste nur location
    int goalY, goalX;
    goalY = rand() % ROWS;
    goalX = rand() % COLS;
    
    GAMEBOARD[goalY][goalX].type = TYPE_GOAL;
    // ints
    int runnerCount = 1;
    int hunterCount = 1;
    int wallCount;
    char difficulty;
    int firstTimeRunner = 1;
    int firstTimeHunter = 1;
    int firstTimeWall = 1;
redo1:
    // runner count + placement
    if (firstTimeRunner)
    {
        while (1)
        {
            printf("HOW MANY RUNNERS: "); scanf("%d", &runnerCount); printf("\n");
            if (runnerCount<=(int)(sqrt(h*w)-2)-1 && runnerCount>0) break;
            printf("\nMAXIMUM RUNNERS: %d\n", (int)(sqrt(h*w)-2)-1);
        }
    }
    firstTimeRunner = 0;
    while (1)
    {
        for (int i = 0; i!=ROWS; i++)
        {
            for (int j = 0; j!=COLS; j++)
            {
                if (GAMEBOARD[i][j].type==TYPE_RUNNER) GAMEBOARD[i][j].type=TYPE_FREE_WAY;
            }
        }
        for (int i = 0; i!=runnerCount; i++)
        {
            int runnerI; int runnerJ;
            while (1)
            {
                runnerI = rand()%ROWS;
                runnerJ = rand()%COLS;
                // bayad ru FREEWAY bashe
                if (GAMEBOARD[runnerI][runnerJ].type != TYPE_FREE_WAY) continue;
                else break;
            }
            GAMEBOARD[runnerI][runnerJ].type = TYPE_RUNNER;
        }
        int positions[2*runnerCount];
        int positionsCursor = 0;
        for (int i = 0; i!=ROWS; i++)
        {
            for (int j = 0; j!=COLS; j++)
            {
                if (GAMEBOARD[i][j].type == TYPE_RUNNER)
                {
                    positions[positionsCursor++] = i;
                    positions[positionsCursor++] = j;
                }
            }
        }
        int distOk = 1;
        // 3
        // [0, 0, 2, 4, 2, 2]
        for (int i = 0; i<2*runnerCount && distOk; i+=2)
        {
            for (int j = 2; j<2*runnerCount; j+=2)
            {
                if (eucDist(positions[i], positions[i+1], positions[i+j], positions[i+j+1])<2)
                {
                    distOk = 0;
                    break;
                }
            }
        }
        if (distOk) break;
    }
    // hunter count + placement
    if (firstTimeHunter)
    {
        while (1)
        {
            printf("HOW MANY HUNTERS: "); scanf("%d", &hunterCount); printf("\n");
            if (hunterCount<=(int)(sqrt(h*w)-2)-1 && hunterCount>0) break;
            printf("\nMAXIMUM HUNTERS: %d\n", (int)(sqrt(h*w)-2)-1);
        }
        scanf("%c", &difficulty);
        difficulty = 'h';
        while (1)
        {
            printf("\n\n\n\n\nCHOOSE DIFFICULTY ('e' for easy, 'h' for hard): "); scanf("%c", &difficulty); printf("\n");
            if (difficulty == 'e' || difficulty == 'h') break;
            printf("\nCHOOSE 'e' for easy, or 'h' for hard\n");
        }
    }
    firstTimeHunter = 0;
    while (1)
    {
        for (int i = 0; i!=ROWS; i++)
        {
            for (int j = 0; j!=COLS; j++)
            {
                if (GAMEBOARD[i][j].type==TYPE_HUNTER) GAMEBOARD[i][j].type=TYPE_FREE_WAY;
            }
        }
        for (int i = 0; i!=hunterCount; i++)
        {
            int hunterI; int hunterJ;
            while (1)
            {
                hunterI = rand()%ROWS;
                hunterJ = rand()%COLS;
                if (GAMEBOARD[hunterI][hunterJ].type != TYPE_FREE_WAY) continue;
                else break;
            }
            GAMEBOARD[hunterI][hunterJ].type = TYPE_HUNTER;
        }
        int positions[2*hunterCount];
        int positionsCursor = 0;
        for (int i = 0; i!=ROWS; i++)
        {
            for (int j = 0; j!=COLS; j++)
            {
                if (GAMEBOARD[i][j].type == TYPE_HUNTER)
                {
                    positions[positionsCursor++] = i;
                    positions[positionsCursor++] = j;
                }
            }
        }
        int distOk = 1;
        // 3
        // [0, 0, 2, 4, 2, 2]
        for (int i = 0; i<2*hunterCount && distOk; i+=2)
        {
            for (int j = 2; j<2*hunterCount; j+=2)
            {
                if (eucDist(positions[i], positions[i+1], positions[i+j], positions[i+j+1])<2)
                {
                    distOk = 0;
                    break;
                }
            }
        }
        if (distOk) break;
    }
    
    // wall count + placement
    /* NOTE 1.2 */
    if (firstTimeWall)
    {
        int wallTries = 0;
        while (1)
        {
            printf("HOW MANY WALLS: "); scanf("%d", &wallCount);
            if (wallCount<=(int)(0.6*ROWS*COLS-1)-15 && wallCount>=0) break;
            if (wallTries==0)
                printf("\n\n\n\n\n\n\n\n\nMAXIMUM WALLS: %d\n\n", (int)(0.6*ROWS*COLS-1)-18);
            else if (wallTries==1)
            {
                printf("\033[0;31m");
                printf("\n\n\n\n\n\n\n\n\nBRO YOU GOOD? I SAID MAXIMUM WALLS: %d\n\n", (int)(0.6*ROWS*COLS-1)-18);
                printf("\033[0m");
            }
            else if (wallTries==2)
            {
                printf("\033[0;31m");
                printf("\n\n\n\n\n\n\n\n\nOUTTA HERE\n\n");
                printf("\033[0m");
                sleep(2);
                return 0;
            }
            wallTries++;
        }
    }
    firstTimeWall = 0;
    while (1) {
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                if (hWalls[i][j] || vWalls[i][j])
                {
                    hWalls[i][j] = 0;
                    vWalls[i][j] = 0;
                }
            }
        }

        int placed = 0;
        while (placed!=wallCount)
        {
            int r = rand()%ROWS;
            int c = rand()%COLS;
            int t = rand()%2;

            if (GAMEBOARD[r][c].type == TYPE_FREE_WAY && !hWalls[r][c] && !vWalls[r][c])
            {
                if (t) hWalls[r][c] = 1;
                else vWalls[r][c] = 1;
                placed++;
            }
        }
        if (isOk(ROWS, COLS, GAMEBOARD, hWalls, vWalls, maxTempWalls, tempWalls)) break;
    }
    // final check
    Point runners[runnerCount];
    int runnerCursor = 0;
    Point hunters[hunterCount];
    int hunterCursor = 0;
    for (int i = 0; i!=ROWS; i++)
    {
        for (int j = 0; j!=COLS; j++)
        {
            if (GAMEBOARD[i][j].type == TYPE_RUNNER)
            {
                runners[runnerCursor].x = j;
                runners[runnerCursor].y = i;
                runnerCursor++;
            }
            else if (GAMEBOARD[i][j].type == TYPE_HUNTER)
            {
                hunters[hunterCursor].x = j;
                hunters[hunterCursor].y = i;
                hunterCursor++;
            }
        }
    }

    int distOk = 1;
    for (int i =0; i!=runnerCount; i++)
    {
        // x < 2 nabashad
        if (eucDist(runners[i].x, runners[i].y, goalX, goalY) < 2) distOk = 0;

        for (int j = 0; j!=hunterCount; j++)
        {
            // x < 2 nabashad
            if (eucDist(runners[i].x, runners[i].y, hunters[j].x, hunters[j].y) < 2) distOk = 0;
            // x < 2 nabashad
            if (eucDist(hunters[j].x, hunters[j].y, goalX, goalY) < 2) distOk = 0;
        }
        if (!distOk) break;
    }
    if (!distOk) goto redo1;
    // end final check
    // END TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP
    // init
    const int screenWidth = w*ONE_UNIT;
    const int screenHeight = h*ONE_UNIT;
    InitWindow(screenWidth, screenHeight, "RUNNERS VS HUNTERS");
    SetTargetFPS(MAX_FPS);

    // set players
    Player players[runnerCount];
    {
        int pCursor = 0;
        for (int i = 0; i!=ROWS; i++)
        {
            for (int j = 0; j!=COLS; j++)
            {
                if (GAMEBOARD[i][j].type == TYPE_RUNNER)
                {
                    Player newP = {i, j, STATE_ALIVE};
                    players[pCursor++] = newP;
                }
            }
        }
    }
    // end set players

    // DEBUG
    // for (int i = 0; i!=ROWS; i++)
    // {
    //     for (int j = 0; j!=COLS; j++)
    //     {
    //         if (!hWalls[i][j] && !vWalls[i][j])
    //             printf("%c ", GAMEBOARD[i][j].type);
    //         if (hWalls[i][j])
    //     }
    //     printf("\n");
    // }
    // END DEBUG

    int activeRunner = 0;
    int isPlayerTurn = 1;
    spawnPrize(ROWS, COLS, GAMEBOARD, hWalls, vWalls, maxTempWalls, tempWalls, &prize);
    while (!WindowShouldClose())
    {
        // inputs
        if (isPlayerTurn && players[activeRunner].state != STATE_ALIVE)
        {
            activeRunner++;
            if (activeRunner >= runnerCount)
            {
                isPlayerTurn = 0;
                activeRunner = 0;
            }
        }
        if (isPlayerTurn && players[activeRunner].state == STATE_ALIVE)
        {
            int i = activeRunner;
            int moved = 0;
            int gotExtraTurn = 0;

            if (IsKeyPressed(KEY_W))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'W', maxTempWalls, tempWalls))
                {
                    if (GAMEBOARD[players[i].i-1][players[i].j].type == TYPE_GOAL)
                    {
                        players[i].state = STATE_WON;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        moved = 1;
                    }
                    else
                    {
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        players[i].i--;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_RUNNER;
                        moved = 1;
                    }
                } else warnMove();
            } 
            else if (IsKeyPressed(KEY_A))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'A', maxTempWalls, tempWalls))
                {
                    if (GAMEBOARD[players[i].i][players[i].j-1].type == TYPE_GOAL)
                    {
                        players[i].state = STATE_WON;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        moved = 1;
                    }
                    else
                    {
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        players[i].j--;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_RUNNER;
                        moved = 1;
                    }
                } else warnMove();
            } 
            else if (IsKeyPressed(KEY_S))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'S', maxTempWalls, tempWalls))
                {
                    if (GAMEBOARD[players[i].i+1][players[i].j].type == TYPE_GOAL)
                    {
                        players[i].state = STATE_WON;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        moved = 1;
                    }
                    else
                    {
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        players[i].i++;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_RUNNER;
                        moved = 1;
                    }
                } else warnMove();
            } 
            else if (IsKeyPressed(KEY_D))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'D', maxTempWalls, tempWalls))
                {
                    if (GAMEBOARD[players[i].i][players[i].j+1].type == TYPE_GOAL)
                    {
                        players[i].state = STATE_WON;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        moved = 1;
                    }
                    else
                    {
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_FREE_WAY;
                        players[i].j++;
                        GAMEBOARD[players[i].i][players[i].j].type = TYPE_RUNNER;
                        moved = 1;
                    }
                } else warnMove();
            }
            // tempwalls
            else if (IsKeyPressed(KEY_UP))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'W', maxTempWalls, tempWalls))
                {
                    int curs;
                    for (curs = 0; tempWalls[curs].i!=-1 && curs!=maxTempWalls; curs++);
                    if (curs==maxTempWalls)
                        warnWall(CUZ_LIMIT_REACHED);
                    else if (GAMEBOARD[players[i].i-1][players[i].j].type == TYPE_GOAL)
                        warnWall(CUZ_INVALID_PLACE);
                    else
                    {
                        TempWall newTW = {players[i].i-1, players[i].j, TYPE_WALL_H, 0};
                        int lezGo = 1;
                        for (int i = 0; i!=maxTempWalls; i++)
                        {
                            if (tempWalls[i].i == newTW.i && tempWalls[i].j == newTW.j && tempWalls[i].type == newTW.type)
                            {
                                warnWall(CUZ_INVALID_PLACE);
                                lezGo = 0;
                            }
                            if (!lezGo) break;
                        }
                        if (lezGo)
                        {
                            tempWalls[curs] = newTW;
                            moved = 1;
                        }
                    }
                } else warnMove();
            }
            else if (IsKeyPressed(KEY_LEFT))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'A', maxTempWalls, tempWalls))
                {
                    int curs;
                    for (curs = 0; tempWalls[curs].i!=-1 && curs!=maxTempWalls; curs++);
                    if (curs==maxTempWalls)
                        warnWall(CUZ_LIMIT_REACHED);
                    else if (GAMEBOARD[players[i].i][players[i].j-1].type == TYPE_GOAL)
                        warnWall(CUZ_INVALID_PLACE);
                    else
                    {
                        TempWall newTW = {players[i].i, players[i].j-1, TYPE_WALL_V, 0};
                        int lezGo = 1;
                        for (int i = 0; i!=maxTempWalls; i++)
                        {
                            if (tempWalls[i].i == newTW.i && tempWalls[i].j == newTW.j && tempWalls[i].type == newTW.type)
                            {
                                warnWall(CUZ_INVALID_PLACE);
                                lezGo = 0;
                            }
                            if (!lezGo) break;
                        }
                        if (lezGo)
                        {
                            tempWalls[curs] = newTW;
                            moved = 1;
                        }
                    }
                } else warnMove();
            }
            else if (IsKeyPressed(KEY_DOWN))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'S', maxTempWalls, tempWalls))
                {
                    int curs;
                    for (curs = 0; tempWalls[curs].i!=-1 && curs!=maxTempWalls; curs++);
                    if (curs==maxTempWalls)
                        warnWall(CUZ_LIMIT_REACHED);
                    else if (GAMEBOARD[players[i].i+1][players[i].j].type == TYPE_GOAL)
                        warnWall(CUZ_INVALID_PLACE);
                    else
                    {
                        TempWall newTW = {players[i].i, players[i].j, TYPE_WALL_H, 0};
                        int lezGo = 1;
                        for (int i = 0; i!=maxTempWalls; i++)
                        {
                            if (tempWalls[i].i == newTW.i && tempWalls[i].j == newTW.j && tempWalls[i].type == newTW.type)
                            {
                                warnWall(CUZ_INVALID_PLACE);
                                lezGo = 0;
                            }
                            if (!lezGo) break;
                        }
                        if (lezGo)
                        {
                            tempWalls[curs] = newTW;
                            moved = 1;
                        }
                    }
                } else warnMove();
            }
            else if (IsKeyPressed(KEY_RIGHT))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, players[i].i, players[i].j, 'D', maxTempWalls, tempWalls))
                {
                    int curs;
                    for (curs = 0; tempWalls[curs].i!=-1 && curs!=maxTempWalls; curs++);
                    if (curs==maxTempWalls)
                        warnWall(CUZ_LIMIT_REACHED);
                    else if (GAMEBOARD[players[i].i][players[i].j+1].type == TYPE_GOAL)
                        warnWall(CUZ_INVALID_PLACE);
                    else
                    {
                        TempWall newTW = {players[i].i, players[i].j, TYPE_WALL_V, 0};
                        int lezGo = 1;
                        for (int i = 0; i!=maxTempWalls; i++)
                        {
                            if (tempWalls[i].i == newTW.i && tempWalls[i].j == newTW.j && tempWalls[i].type == newTW.type)
                            {
                                warnWall(CUZ_INVALID_PLACE);
                                lezGo = 0;
                            }
                            if (!lezGo) break;
                        }
                        if (lezGo)
                        {
                            tempWalls[curs] = newTW;
                            moved = 1;
                        }
                    }
                } else warnMove();
            }
            else if (IsKeyPressed(KEY_SPACE))
                moved = 1;

            // PRIZE CHECK
            if (moved && prize.active && players[i].state == STATE_ALIVE && players[i].i == prize.i && players[i].j == prize.j)
            {
                prize.active = 0;

                int effect = rand() % 3;
                if (effect == 0)
                {
                    gotExtraTurn = 1;
                    showText("PRIZE: ANOTHER TURN!", YELLOW);
                }
                else if (effect == 1)
                {
                    maxTempWalls += 2;
                    if (maxTempWalls > tempWallsCap) maxTempWalls = tempWallsCap;
                    showText("PRIZE: TEMP WALL LIMIT +2!", YELLOW);
                }
                else
                {
                    showText("PRIZE: !!! EARTHQUAKE !!!", YELLOW);
                    earthquake(ROWS, COLS, GAMEBOARD, hWalls, vWalls, runnerCount, players, maxTempWalls, tempWalls, goalY, goalX);
                }

                spawnPrize(ROWS, COLS, GAMEBOARD, hWalls, vWalls, maxTempWalls, tempWalls, &prize);
            }
            if (moved)
            {
                if (!gotExtraTurn)
                {
                    activeRunner++;
                    if (activeRunner >= runnerCount)
                    {
                        isPlayerTurn = 0;
                        activeRunner = 0;
                    }
                }
            }
        }

        // logic
        int gameState = logic(ROWS, COLS, GAMEBOARD, hWalls, vWalls, &isPlayerTurn, maxTempWalls, tempWalls, runnerCount, players, difficulty);
        if (gameState == 1 || gameState == -1)
            goto done;

        // render
        BeginDrawing();
        ClearBackground(BLACK);

        drawMap(ROWS, COLS, GAMEBOARD, hWalls, vWalls, maxTempWalls, tempWalls, runnerCount, players, activeRunner, prize);

        EndDrawing();
    }

done:    
    CloseWindow();
    free(tempWalls);
    return 0;
}


/* NOTE 1.0 */
/* ma az karbar "w" va "h" ro migirim. matrixi ke be vojud miarim tabe e (2n-1) ast (be jaye "n", 
"w" ya "h" mishine). chera? chon darim: n ta block + (n-1) ta divar */

/* NOTE 1.1 */
/* farzan w=3 va h=3, matrix chandXchand? 5X5. hala agar karbar bege "runner" bere (1, 1), tu matrix
darvaghe khahad raft (2, 2). agar bege (2, 2)? khahad raft (4, 4). banabarin tabe e (2n) dar nazar migirim */

/* NOTE 1.2 */
/* darbareye WALL ha... agar karbar bege (1, 1). tebghe NOTE 1.1 mire (2, 2) VALI... ma migim (2+1, 2+1). chera?
soal injuri khaste :). bad tabe initWall ro seda mikonim ok mikone AISDJOAWJIDOAWIJDWOISJOAIJDOWAJIODJ */

/* NOTE 1.3 */
/* age H bud yeki chap, age V yeki bala */

/* NOTE 1.4 */
/* FAGHAT GHABLE initWall MA AZ "TYPE_WALL_W" va "TYPE_WALL_H" ESTEFADE MIKONIM!!! tu initWall (ba tabaeiat az
NOTE 1.3) jashun ro ok mikonim. baraye detection va ... hame majburim joft check konim */

/* NOTE 2.0 */
/* vaghti matrix ok shod, tabe drawMap ro seda mizanim va tamam */

/* NOTE 2.1 */
/* agar TYPE_FREE_WAY ma i va j hayi ke fard bashe ro khat mahv migirim */

/* MOVAGHATAN TABE initWall ESTEFADE !!!!NASHAVAD!!!! */