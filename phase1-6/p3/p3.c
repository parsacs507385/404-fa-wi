#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <raylib.h>

// CONSTS
#define ONE_UNIT 70
#define ONE_LINE_UNIT 4
#define MAX_FPS 60
// TYPES
#define TYPE_HUNTER 'h'
#define TYPE_WALL 'w'
#define TYPE_WALL_H '_'
#define TYPE_WALL_V '|'
#define TYPE_FREE_WAY 'f'
#define TYPE_RUNNER 'r'
#define TYPE_GOAL 'g'
// COLORS
#define COLOR_FREE_WAY BLACK
#define COLOR_NOT_WALL WHITE
#define COLOR_WALL MAGENTA
#define COLOR_RUNNER BLUE
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

int boundCheck(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int playerI, int playerJ, char where)
{
    switch (where)
    {
        case 'W':
            if (playerI==0) return 0;
            if (hWalls[playerI-1][playerJ]) return 0;
            if (gameboard[playerI-1][playerJ].type == TYPE_FREE_WAY || gameboard[playerI-1][playerJ].type == TYPE_GOAL)
                return 1;
            return 0;
            break;
        case 'A':
            if (playerJ==0) return 0;
            if (vWalls[playerI][playerJ-1]) return 0;
            if (gameboard[playerI][playerJ-1].type == TYPE_FREE_WAY || gameboard[playerI][playerJ-1].type == TYPE_GOAL)
                return 1;
            return 0;
            break;
        case 'S':
            if (playerI==rows-1) return 0;
            if (hWalls[playerI][playerJ]) return 0;
            if (gameboard[playerI+1][playerJ].type == TYPE_FREE_WAY || gameboard[playerI+1][playerJ].type == TYPE_GOAL)
                return 1;
            return 0;
            break;
        case 'D':
            if (playerJ==cols-1) return 0;
            if (vWalls[playerI][playerJ]) return 0;
            if (gameboard[playerI][playerJ+1].type == TYPE_FREE_WAY || gameboard[playerI][playerJ+1].type == TYPE_GOAL)
                return 1;
            return 0;
            break;
    }
}

int didWin(int rows, int cols, Entity gameboard[rows][cols])
{
    int v_didWin = 1;
    for (int i = 0; i!=rows; i++)
        for (int j = 0; j!=cols; j++)
            if (gameboard[i][j].type == TYPE_GOAL)
                v_didWin = 0;
    return v_didWin;
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

void moveHunters(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int iPlayer, int jPlayer)
{
    int iMe;
    int jMe;
    // find self
        for (int i = 0; i!=rows; i++)
        {
            for (int j = 0; j!=cols; j++)
            {
                if (gameboard[i][j].type==TYPE_HUNTER)
                {
                    iMe = i;
                    jMe = j;
                }
            }
        }
    // END find self
    
    for (int moveCount = 0; moveCount < 2; moveCount++)
    {
        // if must go right
            if (jPlayer>jMe)
            {
                // should move directly
                if (!vWalls[iMe][jMe] && (gameboard[iMe][jMe+1].type == TYPE_FREE_WAY || gameboard[iMe][jMe+1].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe][jMe+1].type = TYPE_HUNTER;
                    jMe++;
                    continue;
                }
                // shouldnt move
                if (iMe==iPlayer)
                    return;
                // should move indirectly
                if (iMe>iPlayer)
                {
                    if (iMe != 0 && !hWalls[iMe-1][jMe] && (gameboard[iMe-1][jMe].type == TYPE_FREE_WAY || gameboard[iMe-1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe-1][jMe].type = TYPE_HUNTER;
                        iMe--;
                        continue;
                    }
                }
                else
                {
                    if (iMe != rows-1 && !hWalls[iMe][jMe] && (gameboard[iMe+1][jMe].type == TYPE_FREE_WAY || gameboard[iMe+1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe+1][jMe].type = TYPE_HUNTER;
                        iMe++;
                        continue;
                    }
                }
            }
        // if must go left
            if (jMe>jPlayer)
            {
                // should move directly
                if (!vWalls[iMe][jMe-1] && (gameboard[iMe][jMe-1].type == TYPE_FREE_WAY || gameboard[iMe][jMe-1].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe][jMe-1].type = TYPE_HUNTER;
                    jMe--;
                    continue;
                }
                // shouldnt move
                if (iMe==iPlayer)
                    return;
                // should move indirectly
                if (iMe>iPlayer)
                {
                    if (iMe != 0 && !hWalls[iMe-1][jMe] && (gameboard[iMe-1][jMe].type == TYPE_FREE_WAY || gameboard[iMe-1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe-1][jMe].type = TYPE_HUNTER;
                        iMe--;
                        continue;
                    }
                }
                else
                {
                    if (iMe != rows-1 && !hWalls[iMe][jMe] && (gameboard[iMe+1][jMe].type == TYPE_FREE_WAY || gameboard[iMe+1][jMe].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe+1][jMe].type = TYPE_HUNTER;
                        iMe++;
                        continue;
                    }
                }
            }
        // END if must go left
        // if must go up
            if (iMe>iPlayer)
            {
                // should move directly
                if (!hWalls[iMe-1][jMe] && (gameboard[iMe-1][jMe].type == TYPE_FREE_WAY || gameboard[iMe-1][jMe].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe-1][jMe].type = TYPE_HUNTER;
                    iMe--;
                    continue;
                }
                // shouldnt move
                if (jMe==jPlayer)
                    return;
                // should move indirectly
                if (jMe>jPlayer)
                {
                    if (jMe != 0 && !vWalls[iMe][jMe-1] && (gameboard[iMe][jMe-1].type == TYPE_FREE_WAY || gameboard[iMe][jMe-1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe-1].type = TYPE_HUNTER;
                        jMe--;
                        continue;
                    }
                }
                else
                {
                    if (jMe != cols-1 && !vWalls[iMe][jMe] && (gameboard[iMe][jMe+1].type == TYPE_FREE_WAY || gameboard[iMe][jMe+1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe+1].type = TYPE_HUNTER;
                        jMe++;
                        continue;
                    }
                }
            }
        // END if must go up
        // if must go down
            if (iMe<iPlayer)
            {
                // should move directly
                if (!hWalls[iMe][jMe] && (gameboard[iMe+1][jMe].type == TYPE_FREE_WAY || gameboard[iMe+1][jMe].type == TYPE_RUNNER))
                {
                    gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                    gameboard[iMe+1][jMe].type = TYPE_HUNTER;
                    iMe++;
                    continue;
                }
                // shouldnt move
                if (jMe==jPlayer)
                    return;
                // should move indirectly
                if (jMe>jPlayer)
                {
                    if (jMe != 0 && !vWalls[iMe][jMe-1] && (gameboard[iMe][jMe-1].type == TYPE_FREE_WAY || gameboard[iMe][jMe-1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe-1].type = TYPE_HUNTER;
                        jMe--;
                        continue;
                    }
                }
                else
                {
                    if (jMe != cols-1 && !vWalls[iMe][jMe] && (gameboard[iMe][jMe+1].type == TYPE_FREE_WAY || gameboard[iMe][jMe+1].type == TYPE_RUNNER))
                    {
                        gameboard[iMe][jMe].type = TYPE_FREE_WAY;
                        gameboard[iMe][jMe+1].type = TYPE_HUNTER;
                        jMe++;
                        continue;
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

int logic(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols], int* isPlayerTurn)
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
                break;
            }
        }
    }
    // hunters should go
    if (!(*isPlayerTurn))
    {
        moveHunters(rows, cols, gameboard, hWalls, vWalls, iPlayer, jPlayer);
        *isPlayerTurn = 1;
    }

    if (didWin(rows, cols, gameboard))
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

void drawMap(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols])
{
    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            Color cellColor = COLOR_FREE_WAY;
            if (gameboard[i][j].type == TYPE_HUNTER) cellColor = COLOR_HUNTER;
            else if (gameboard[i][j].type == TYPE_RUNNER) cellColor = COLOR_RUNNER;
            else if (gameboard[i][j].type == TYPE_GOAL) cellColor = COLOR_GOAL;
            DrawRectangle(j*ONE_UNIT, i*ONE_UNIT, ONE_UNIT, ONE_UNIT, cellColor);

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
}

float random0n1(int mult)
{
    double randomNumber = (double)rand();
    while (randomNumber>1) randomNumber /= 10;
    // 0.783845621
    randomNumber = (double)((int)(randomNumber*10)/10.0);
    return (float)(randomNumber*mult);
}

int isOk(int rows, int cols, Entity gameboard[rows][cols], int hWalls[rows][cols], int vWalls[rows][cols])
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

            if (ni >= 0 && ni!=rows && nj >= 0 && nj!=cols)
            {
                if (!visited[ni][nj])
                {
                    int canMove = boundCheck(rows, cols, gameboard, hWalls, vWalls, ci, cj, dir[d]);
                    
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

float eucDist(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
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
    int firstTimeRunner = 1;
    int firstTimeHunter = 1;
    int firstTimeWall = 1;
redo1:
    // runner count + placement
    // if (firstTimeRunner)
    // {
    //     while (1)
    //     {
    //         printf("HOW MANY RUNNERS: "); scanf("%d", &runnerCount); printf("\n");
    //         if (runnerCount<sqrt(h*w)-2 && runnerCount>0) break;
    //         printf("\nMAXIMUM RUNNERS: %d\n", (int)(sqrt(h*w)-2)-1);
    //     }
    // }
    // firstTimeRunner = 0;
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
    // // hunter count + placement
    // if (firstTimeHunter)
    // {
    //     while (1)
    //     {
    //         printf("HOW MANY HUNTERS: "); scanf("%d", &hunterCount); printf("\n");
    //         if (hunterCount<sqrt(h*w)-2 && hunterCount>0) break;
    //         printf("\nMAXIMUM HUNTERS: %d\n", (int)(sqrt(h*w)-2)-1);
    //     }
    // }
    // firstTimeHunter = 0;
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
                printf("\n\n\n\n\n\n\n\n\nMAXIMUM WALLS: %d\n\n", (int)(0.6*ROWS*COLS-1)-15);
            else if (wallTries==1)
            {
                printf("\033[0;31m");
                printf("\n\n\n\n\n\n\n\n\nBRO YOU TRIPPIN? I SAID MAXIMUM WALLS: %d\n\n", (int)(0.6*ROWS*COLS-1)-15);
                printf("\033[0m");
            }
            else if (wallTries==2)
            {
                printf("\033[0;31m");
                printf("\n\n\n\n\n\n\n\n\nOUTTA HERE CUZ\n\n");
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
        if (isOk(ROWS, COLS, GAMEBOARD, hWalls, vWalls)) break;
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
    InitWindow(screenWidth, screenHeight, "RUNNER VS HUNTER");
    SetTargetFPS(MAX_FPS);

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

    int isPlayerTurn = 1;
    while (!WindowShouldClose())
    {
        // inputs
        if (isPlayerTurn)
        {
            // find player
            int playerI = -1;
            int playerJ = -1;
            for (int i = 0; i!=ROWS && playerI==-1; i++)
            {
                for (int j = 0; j!=COLS; j++)
                {
                    if (GAMEBOARD[i][j].type == TYPE_RUNNER)
                    {
                        playerI = i;
                        playerJ = j;
                        break;
                    }
                }
            }
            // end find player
            if (IsKeyPressed(KEY_W))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, playerI, playerJ, 'W'))
                {
                    GAMEBOARD[playerI][playerJ].type = TYPE_FREE_WAY;
                    GAMEBOARD[playerI-1][playerJ].type = TYPE_RUNNER;
                    isPlayerTurn = 0;
                }
                else warnMove();
            }
            else if (IsKeyPressed(KEY_A))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, playerI, playerJ, 'A'))
                {
                    GAMEBOARD[playerI][playerJ].type = TYPE_FREE_WAY;
                    GAMEBOARD[playerI][playerJ-1].type = TYPE_RUNNER;
                    isPlayerTurn = 0;
                }
                else warnMove();
            }
            else if (IsKeyPressed(KEY_S))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, playerI, playerJ, 'S'))
                {
                    GAMEBOARD[playerI][playerJ].type = TYPE_FREE_WAY;
                    GAMEBOARD[playerI+1][playerJ].type = TYPE_RUNNER;
                    isPlayerTurn = 0;
                }
                else warnMove();
            }
            else if (IsKeyPressed(KEY_D))
            {
                if (boundCheck(ROWS, COLS, GAMEBOARD, hWalls, vWalls, playerI, playerJ, 'D'))
                {
                    GAMEBOARD[playerI][playerJ].type = TYPE_FREE_WAY;
                    GAMEBOARD[playerI][playerJ+1].type = TYPE_RUNNER;
                    isPlayerTurn = 0;
                }
                else warnMove();
            }
            else if (IsKeyPressed(KEY_SPACE))
            {
                isPlayerTurn = 0;
            }
        }

        // logic
        int gameState = logic(ROWS, COLS, GAMEBOARD, hWalls, vWalls, &isPlayerTurn);
        if (gameState == 1 || gameState == -1)
            return 0;

        // render
        BeginDrawing();
        ClearBackground(BLACK);

        drawMap(ROWS, COLS, GAMEBOARD, hWalls, vWalls);

        EndDrawing();
    }

    CloseWindow();

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