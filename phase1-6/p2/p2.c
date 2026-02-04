#include <stdio.h>
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
#define COLOR_FREE_WAY WHITE
// #define COLOR_NOT_WALL Color{200, 200, 200, 255}
const Color COLOR_NOT_WALL = {200, 200, 200, 255};
#define COLOR_WALL RED
#define COLOR_RUNNER BLUE
#define COLOR_HUNTER RED
#define COLOR_GOAL YELLOW


typedef struct 
{
    char type;
} Entity;
typedef struct
{
    int x;
    int y;
} Point;

// HELPER FUNCs
// void initWall(int rows, int cols, Entity gameboard[rows][cols])
// {
//     for (int i = 0; i<rows; i++)
//     {
//         for (int j = 0; j<cols; j++)
//         {
//             /* NOTE 1.3  && NOTE 1.4 */
//             // wall detected
//             if (gameboard[i][j].type==TYPE_WALL_H)
//             {
//                 gameboard[i][j].type = TYPE_FREE_WAY;
//                 gameboard[i][j-1].type = TYPE_WALL_H;
//             }
//             if (gameboard[i][j].type==TYPE_WALL_V)
//             {
//                 gameboard[i][j].type = TYPE_FREE_WAY;
//                 gameboard[i-1][j].type = TYPE_WALL_V;
//             }
//         }
//     }
// }

void drawMap(int rows, int cols, Entity gameboard[rows][cols])
{
    for (int i = 0; i<rows; i++)
    {
        for (int j = 0; j<cols; j++)
        {
            int wasDoneInSwitch = 0;
            int numUnitsBeforeI = i / 2; int numLinesBeforeI = (i + 1) / 2;
            int offsetI = (numUnitsBeforeI * ONE_UNIT) + (numLinesBeforeI * ONE_LINE_UNIT);
            int numUnitsBeforeJ = j / 2; int numLinesBeforeJ = (j + 1) / 2;
            int offsetJ = (numUnitsBeforeJ * ONE_UNIT) + (numLinesBeforeJ * ONE_LINE_UNIT);
            switch (gameboard[i][j].type)
            {
            case TYPE_HUNTER:
                DrawRectangle(offsetJ, offsetI, ONE_UNIT, ONE_UNIT, COLOR_HUNTER);
                wasDoneInSwitch = 1;
                break;
            
            case TYPE_WALL_V:
                DrawRectangle(offsetJ+ONE_UNIT-ONE_LINE_UNIT, offsetI, ONE_LINE_UNIT, ONE_UNIT, COLOR_WALL);
                wasDoneInSwitch = 1;
                break;
            
            case TYPE_WALL_H:
                DrawRectangle(offsetJ, offsetI+ONE_UNIT-ONE_LINE_UNIT, ONE_UNIT, ONE_LINE_UNIT, COLOR_WALL);
                wasDoneInSwitch = 1;
                break;
            
            case TYPE_RUNNER:
                DrawRectangle(offsetJ, offsetI, ONE_UNIT, ONE_UNIT, COLOR_RUNNER);
                wasDoneInSwitch = 1;
                break;
            
            case TYPE_GOAL:
                DrawRectangle(offsetJ, offsetI, ONE_UNIT, ONE_UNIT, COLOR_GOAL);
                wasDoneInSwitch = 1;
                break;
            }
            // if is freeWay
            /* NOTE 2.1 */
            if (!wasDoneInSwitch)
            {
                int width = (j % 2 == 1) ? ONE_LINE_UNIT : ONE_UNIT;
                int height = (i % 2 == 1) ? ONE_LINE_UNIT : ONE_UNIT;
                DrawRectangle(offsetJ, offsetI, width, height, COLOR_NOT_WALL);
            }
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

int isOk(int rows, int cols, Entity gameboard[rows][cols])
{
    int visited[rows][cols];

    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
            visited[i][j] = 0;
    }

    int startI = -1;
    int startJ = -1;
    for (int i = 0; i!=rows && startI == -1; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (gameboard[i][j].type == TYPE_FREE_WAY)
            {
                startI = i;
                startJ = j;
                break;
            }
        }
    }

    if (startI == -1) return 1;

    int stackI[rows*cols];
    int stackJ[rows*cols];
    int top = 0;

    stackI[top] = startI;
    stackJ[top] = startJ;
    top++;
    visited[startI][startJ] = 1;

    // DFS
    while (top>0)
    {
        top--;
        int ci = stackI[top];
        int cj = stackJ[top];

        int di[4] = {-1, 1, 0, 0};
        int dj[4] = {0, 0, -1, 1};

        for (int d = 0; d!=4; d++)
        {
            int ni = ci + di[d];
            int nj = cj + dj[d];

            if (ni >= 0 && ni!=rows && nj >= 0 && nj!=cols)
            {
                if (!visited[ni][nj] && gameboard[ni][nj].type != TYPE_WALL_H && gameboard[ni][nj].type != TYPE_WALL_V)
                {
                    visited[ni][nj] = 1;
                    stackI[top] = ni;
                    stackJ[top] = nj;
                    top++;
                }
            }
        }
    }

    for (int i = 0; i!=rows; i++)
    {
        for (int j = 0; j!=cols; j++)
        {
            if (gameboard[i][j].type == TYPE_FREE_WAY && !visited[i][j])
                return 0;
        }
    }

    return 1;
}

float eucDist(int x1, int y1, int x2, int y2)
{
    x1 /= 2;
    y1 /= 2;
    x2 /= 2;
    y2 /= 2;
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
    const int ROWS = 2*h-1; const int COLS = 2*w-1;
    Entity GAMEBOARD[ROWS][COLS];
    // init board to TYPE_FREE_WAY
    for (int i = 0; i!=ROWS; i++)
    {
        for (int j = 0; j!=COLS; j++)
        {
            Entity newEntity = {TYPE_FREE_WAY};
            GAMEBOARD[i][j] = newEntity;
        }
    }
    // TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP
    // haste nur location
    int goalY, goalX;
    while (1)
    {
        goalY = rand() % ROWS;
        goalX = rand() % COLS;
        if (goalY%2==0 && goalX%2==0) break;
    }
    
    GAMEBOARD[goalY][goalX].type = TYPE_GOAL;
    // ints
    int runnerCount;
    int hunterCount;
    int wallCount;
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
            if (runnerCount<sqrt(h*w)-2 && runnerCount>0) break;
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
                runnerI = (rand()%h) * 2;
                runnerJ = (rand()%w) * 2;
                // bayad zoj bashe
                if (runnerI%2==1 || runnerJ%2==1) continue;
                    else
                    {
                        // bayad ru FREEWAY bashe
                        if (GAMEBOARD[runnerI][runnerJ].type != TYPE_FREE_WAY) continue;
                        else break;
                    }
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
            if (hunterCount<sqrt(h*w)-2 && hunterCount>0) break;
            printf("\nMAXIMUM HUNTERS: %d\n", (int)(sqrt(h*w)-2)-1);
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
                hunterI = (rand()%h)*2;
                hunterJ = (rand()%w)*2;
                // bayad zoj bashe
                if (hunterI%2==1 || hunterJ%2==1) continue;
                    else
                    {
                        // bayad ru FREEWAY bashe
                        if (GAMEBOARD[hunterI][hunterJ].type != TYPE_FREE_WAY) continue;
                        else break;
                    }
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
        while (1)
        {
            printf("HOW MANY WALLS: "); scanf("%d", &wallCount); printf("\n");
            if (wallCount<0.6*h*w && wallCount>=0) break;
            printf("\nMAXIMUM WALLS: %d\n", 0.6*h*w-1);
        }
    }
    firstTimeWall = 0;
    while (1) {
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                if (GAMEBOARD[i][j].type == TYPE_WALL_H || GAMEBOARD[i][j].type == TYPE_WALL_V) 
                    GAMEBOARD[i][j].type = TYPE_FREE_WAY;
            }
        }

        int placed = 0;
        while (placed!=wallCount)
        {
            int r = rand() % ROWS;
            int c = rand() % COLS;

            if (r%2!=c%2)
            { 
                if (GAMEBOARD[r][c].type == TYPE_FREE_WAY)
                {
                    GAMEBOARD[r][c].type = (r % 2 == 1) ? TYPE_WALL_V : TYPE_WALL_H;
                    placed++;
                }
            }
        }
        if (isOk(ROWS, COLS, GAMEBOARD)) break;
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
    const int screenWidth = w*ONE_UNIT + (w-1)*ONE_LINE_UNIT;
    const int screenHeight = h*ONE_UNIT + (h-1)*ONE_LINE_UNIT;
    InitWindow(screenWidth, screenHeight, "RUNNER VS HUNTER");
    SetTargetFPS(MAX_FPS);

    // DEBUG
    for (int i = 0; i!=ROWS; i++)
    {
        for (int j = 0; j!=COLS; j++)
        {
            printf("%c ", GAMEBOARD[i][j].type);
        }
        printf("\n");
    }
    // END DEBUG


    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);


        drawMap(ROWS, COLS, GAMEBOARD);

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