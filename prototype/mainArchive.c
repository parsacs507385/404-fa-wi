#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <SDL3/SDL.h>

#define ONE_BLOCK_W 100
#define ONE_BLOCK_H 100

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int score = 0;
void hunterAI(int n, int m, int arr[n][m], int iPlayer, int jPlayer, float* hunterX, float* hunterY)
{
    int iMe;
    int jMe;
    // find self
        for (int i = 0; i!=n; i++)
        {
            for (int j = 0; j!=m; j++)
            {
                if (arr[i][j]==-2)
                {
                    iMe = i;
                    jMe = j;
                }
            }
        }
    // END find self
    
    // if must go right
        if (jPlayer>jMe)
        {
            if (arr[iMe][jMe+1] != -1)
            {
                arr[iMe][jMe] = 0;
                arr[iMe][jMe+1] = -2;
                *hunterX += ONE_BLOCK_W;
                return;
            }
            if (arr[iMe-1][jMe] != -1 && iMe-1 != -1)
            {
                arr[iMe][jMe] = 0;
                arr[iMe-1][jMe] = -2;
                *hunterY -= ONE_BLOCK_H;
                return;
            }
            if (arr[iMe+1][jMe] != -1 && iMe+1 != n)
            {
                arr[iMe][jMe] = 0;
                arr[iMe+1][jMe] = -2;
                *hunterY += ONE_BLOCK_H;
                return;
            }
            // wont move
            return;
        }
    // END if must go right
    // if must go left
        if (jPlayer<jMe)
        {
            if (arr[iMe][jMe-1] != -1)
            {
                arr[iMe][jMe] = 0;
                arr[iMe][jMe-1] = -2;
                *hunterX -= ONE_BLOCK_W;
                return;
            }
            if (arr[iMe-1][jMe] != -1 && iMe-1 != -1)
            {
                arr[iMe][jMe] = 0;
                arr[iMe-1][jMe] = -2;
                *hunterY -= ONE_BLOCK_H;
                return;
            }
            if (arr[iMe+1][jMe] != -1 && iMe+1 != n)
            {
                arr[iMe][jMe] = 0;
                arr[iMe+1][jMe] = -2;
                *hunterY += ONE_BLOCK_H;
                return;
            }
            // wont move
            return;
        }
    // END if must go left
    // if must go up
        if (iPlayer<iMe)
        {
            if (arr[iMe-1][jMe] != -1)
            {
                arr[iMe][jMe] = 0;
                arr[iMe-1][jMe] = -2;
                *hunterY -= ONE_BLOCK_H;
                return;
            }
            // wont move
            return;
        }
    // END if must go up
    // if must go down
        if (iPlayer>iMe)
        {
            if (arr[iMe+1][jMe] != -1)
            {
                arr[iMe][jMe] = 0;
                arr[iMe+1][jMe] = -2;
                *hunterY += ONE_BLOCK_H;
                return;
            }
            // wont move
            return;
        }
    // END if must go down
    return;
}

int didLose(int n, int m, int arr[n][m])
{
    for (int i = 0; i!=n; i++)
    {
        for (int j = 0; j!=m; j++)
        {
            if (arr[i][j] == 1) return 0;
        }
    }

    return 1;
}

int didWin(int i, int j, int iWin, int jWin)
{
    if (i == iWin && j == jWin)
    {
        return 1;
    }
    return 0;
}

void updatePlayer(SDL_FRect* player, float playerX, float playerY)
{
    player->x = playerX; player->y = playerY;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, player);
}

void updateEntity(SDL_FRect* entity, float i, float j)
{
    entity->x = j; entity->y = i;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderRect(renderer, entity);
}

void updateBlocks(int n, int m, SDL_FRect arr[n*m])
{
    for (int i = 0; i!=n; i++)
    {
        if (arr[i].w != 0)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderRect(renderer, &arr[i]);
        }
    }
}

void updateGoal(SDL_FRect goal)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderRect(renderer, &goal);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { return 1; }
    // TRY TO LOAD SCORE
    int storedScore = 0;
    char storedScoreBuffer[100];
    FILE* preFile = fopen("save.txt", "r");
    FILE* newFile;
    if (preFile == NULL)
    {
        fclose(preFile);
        newFile = fopen("save.txt", "w");
        fprintf(newFile, "0");
        storedScore = 0;
        fclose(newFile);
        score = storedScore;
    }
    else
    {
        fgets(storedScoreBuffer, 100, preFile);
        storedScore = atoi(storedScoreBuffer);
        fclose(preFile);
        score = storedScore;
    }
    // END TRY TO LOAD SCORE
start:
    srand(time(NULL));

    // INPUTS
        int n;
        int m;
        while (1)
        {
            printf("ENTER n (n in {6, 7, 8, 9}): ");
            scanf("%d", &n);
            // printf("\nENTER m (m in {6, 7, 8, 9}): ");
            // scanf("%d", &m);
            m=n;

            if (n<6 || n>9 || m<6 || m>9)
            {
                printf("\nhmm...\n\n");
            }
            else
            {
                break;
            }
        }
    // END INPUTS

    // MATRICES
        // -2: hunter, -1: wall, 0: freeSpace, 1: player
        int playerPositionHunterMatrixI = n-1;
        int playerPositionHunterMatrixJ = m-1;
        int hunterPositionHunterMatrixI = 0;
        int hunterPositionHunterMatrixJ = 0;
        int hunterMatrix[n][m];
        for (int i = 0; i!=n; i++)
        {
            for (int j = 0; j!=m; j++)
            {
                hunterMatrix[i][j] = 0;
            }
        }
        hunterMatrix[0][0] = -2;
        hunterMatrix[playerPositionHunterMatrixI][playerPositionHunterMatrixJ] = 1;

        // -1: wall, 0: freeSpace, 1: win
        const int winPositionPlayerMatrixI = ((double)rand()/(double)RAND_MAX)*(2);
        const int winPositionPlayerMatrixJ = ((double)rand()/(double)RAND_MAX)*(4)+1;
        int playerMatrix[n][m];
        for (int i = 0; i!=n; i++)
        {
            for (int j = 0; j!=m; j++)
            {
                playerMatrix[i][j] = 0;
            }
        }
        playerMatrix[winPositionPlayerMatrixI][winPositionPlayerMatrixJ] = 1;
    // END MATRICES


    window = SDL_CreateWindow("TALE OF NIGGERS", n*ONE_BLOCK_W, m*ONE_BLOCK_H, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
    
    // CONSTS
        float PLAYER_W = ONE_BLOCK_W;
        float PLAYER_H = ONE_BLOCK_H;
    // END CONSTS

    // GLOBALS
        float playerX = (playerPositionHunterMatrixJ)*ONE_BLOCK_W;
        float playerY = (playerPositionHunterMatrixI)*ONE_BLOCK_H;
        SDL_FRect* player = (SDL_FRect*)malloc(sizeof(SDL_FRect));
        player->x = playerX; player->y = playerY; player->w = PLAYER_W; player->h = PLAYER_H;
        
        float hunterX = (hunterPositionHunterMatrixJ)*ONE_BLOCK_W;
        float hunterY = (hunterPositionHunterMatrixI)*ONE_BLOCK_H;
        SDL_FRect* hunter = (SDL_FRect*)malloc(sizeof(SDL_FRect));
        hunter->x = hunterX; hunter->y = hunterY; hunter->w = ONE_BLOCK_W; hunter->h = ONE_BLOCK_H;

        SDL_FRect blocks[n*m];
        int blockCounter = 0;
        for (int i = 0; i!=n*m; i++)
        {
            blocks[i].x = 0.0f;
            blocks[i].y = 0.0f;
            blocks[i].w = 0.0f;
            blocks[i].h = 0.0f;
        }

        SDL_FRect goal = {winPositionPlayerMatrixJ*ONE_BLOCK_W, winPositionPlayerMatrixI*ONE_BLOCK_H, ONE_BLOCK_W, ONE_BLOCK_H};
    // END GLOBALS

    SDL_Event event;
    int isPlayerTurn = 1;
    int isRunning = 1;
    while (isRunning)
    {
        // GET LOCATION IN HUNTERMATRIX
        int iPlayer;
        int jPlayer;
        for (int i = 0; i!=n; i++)
        {
            for (int j = 0; j!=m; j++)
            {
                if (hunterMatrix[i][j] == 1)
                {
                    iPlayer = i;
                    jPlayer = j;
                }
            }
        }
        // KEYBOARD
        while (SDL_PollEvent(&event))
        {   
            // END GET LOCATION IN HUNTERMATRIX
            if (event.type == SDL_EVENT_QUIT) { isRunning = 0; }
            // KEYBINDS
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (isPlayerTurn)
                {
                    if (event.key.key == SDLK_W && playerY!=0 && hunterMatrix[iPlayer-1][jPlayer]!=-1)
                    {
                        hunterMatrix[iPlayer][jPlayer] = 0;
                        hunterMatrix[iPlayer-1][jPlayer] = 1;
                        playerY -= ONE_BLOCK_H;
                        if (didWin(iPlayer-1, jPlayer, winPositionPlayerMatrixI, winPositionPlayerMatrixJ)) goto won;
                        isPlayerTurn = 0;
                        goto goto1;
                    }
                    else if (event.key.key == SDLK_A && playerX!=0 && hunterMatrix[iPlayer][jPlayer-1]!=-1)
                    {
                        hunterMatrix[iPlayer][jPlayer] = 0;
                        hunterMatrix[iPlayer][jPlayer-1] = 1;
                        playerX -= ONE_BLOCK_W;
                        if (didWin(iPlayer, jPlayer-1, winPositionPlayerMatrixI, winPositionPlayerMatrixJ)) goto won;
                        isPlayerTurn = 0;
                        goto goto1;
                    }
                    else if (event.key.key == SDLK_S && playerY+ONE_BLOCK_H!=m*ONE_BLOCK_H && hunterMatrix[iPlayer+1][jPlayer]!=-1)
                    {
                        hunterMatrix[iPlayer][jPlayer] = 0;
                        hunterMatrix[iPlayer+1][jPlayer] = 1;
                        playerY += ONE_BLOCK_H;
                        if (didWin(iPlayer+1, jPlayer, winPositionPlayerMatrixI, winPositionPlayerMatrixJ)) goto won;
                        isPlayerTurn = 0;
                        goto goto1;
                    }
                    else if (event.key.key == SDLK_D && playerX+ONE_BLOCK_W!=n*ONE_BLOCK_W && hunterMatrix[iPlayer][jPlayer+1]!=-1)
                    {
                        hunterMatrix[iPlayer][jPlayer] = 0;
                        hunterMatrix[iPlayer][jPlayer+1] = 1;
                        playerX += ONE_BLOCK_W;
                        if (didWin(iPlayer, jPlayer, winPositionPlayerMatrixI, winPositionPlayerMatrixJ)) goto won;
                        isPlayerTurn = 0;
                        goto goto1;
                    }
                    // BLOCK PLACEMENT
                    else if (event.key.key == SDLK_UP && playerY!=0 && hunterMatrix[iPlayer-1][jPlayer]!=-1 && hunterMatrix[iPlayer-1][jPlayer]!=-2)
                    {
                        hunterMatrix[iPlayer-1][jPlayer] = -1;
                        isPlayerTurn = 0;

                        blocks[blockCounter].x = (jPlayer)*ONE_BLOCK_W;
                        blocks[blockCounter].y = (iPlayer-1)*ONE_BLOCK_H;
                        blocks[blockCounter].w = ONE_BLOCK_W;
                        blocks[blockCounter].h = ONE_BLOCK_H;
                        blockCounter++;

                        goto goto1;
                    }
                    else if (event.key.key == SDLK_LEFT && playerX!=0 && hunterMatrix[iPlayer][jPlayer-1]!=-1 && hunterMatrix[iPlayer][jPlayer-1]!=-2)
                    {
                        hunterMatrix[iPlayer][jPlayer-1] = -1;
                        isPlayerTurn = 0;

                        blocks[blockCounter].x = (jPlayer-1)*ONE_BLOCK_W;
                        blocks[blockCounter].y = (iPlayer)*ONE_BLOCK_H;
                        blocks[blockCounter].w = ONE_BLOCK_W;
                        blocks[blockCounter].h = ONE_BLOCK_H;
                        blockCounter++;

                        goto goto1;
                    }
                    else if (event.key.key == SDLK_DOWN && playerY+ONE_BLOCK_H!=m*ONE_BLOCK_H && hunterMatrix[iPlayer+1][jPlayer]!=-1 && hunterMatrix[iPlayer+1][jPlayer]!=-2)
                    {
                        hunterMatrix[iPlayer+1][jPlayer] = -1;
                        isPlayerTurn = 0;

                        blocks[blockCounter].x = (jPlayer)*ONE_BLOCK_W;
                        blocks[blockCounter].y = (iPlayer+1)*ONE_BLOCK_H;
                        blocks[blockCounter].w = ONE_BLOCK_W;
                        blocks[blockCounter].h = ONE_BLOCK_H;
                        blockCounter++;

                        goto goto1;
                    }
                    else if (event.key.key == SDLK_RIGHT && playerX+ONE_BLOCK_W!=n*ONE_BLOCK_W && hunterMatrix[iPlayer][jPlayer+1]!=-1 && hunterMatrix[iPlayer][jPlayer+1]!=-2)
                    {
                        hunterMatrix[iPlayer][jPlayer+1] = -1;
                        isPlayerTurn = 0;

                        blocks[blockCounter].x = (jPlayer+1)*ONE_BLOCK_W;
                        blocks[blockCounter].y = (iPlayer)*ONE_BLOCK_H;
                        blocks[blockCounter].w = ONE_BLOCK_W;
                        blocks[blockCounter].h = ONE_BLOCK_H;
                        blockCounter++;

                        goto goto1;
                    }
goto1:
                }
            }
        }

        // HUNTER AI
        if (!isPlayerTurn)
        {
            if (didLose(n, m, hunterMatrix)) goto lost;
            hunterAI(n, m, hunterMatrix, iPlayer, jPlayer, &hunterX, &hunterY);
            if (didLose(n, m, hunterMatrix)) goto lost;
            isPlayerTurn = 1;
        }
        // END HUNTER AI
        // START RENDER
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);
        // OBJECTS
        
        updatePlayer(player, playerX, playerY);
        updateEntity(hunter, hunterY, hunterX);
        updateBlocks(n, m, blocks);
        updateGoal(goal);

        // ENDOBJECTS

        
        SDL_RenderPresent(renderer);
        // FPS
        SDL_Delay(1000 / 60);
    }

lost:
    // SCORE
    score = (score-(int)(((double)rand()/(double)RAND_MAX)*200)<0) ? 0 : score-(int)(((double)rand()/(double)RAND_MAX)*200);
    // FREE
    free(player);
    free(hunter);
    // CLEAN
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("\n\n\n--------------YOU LOST--------------\n\nSCORE: %d\n\n\n ('r' to restart, else to exit)\n\n\n", score);
    char want = getch();
    if (want == 'r')
    {
        goto start;
    }
    // SAVE
    FILE* fp = fopen("save.txt", "w");
    char fpBuffer[100];
    sprintf(fpBuffer, "%d", score);
    fprintf(fp, fpBuffer);
    fclose(fp);
    return 0;
won:
    // SCORE
    score += (int)(((double)rand()/(double)RAND_MAX)*200);
    // FREE
    free(player);
    free(hunter);
    // CLEAN
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("\n\n\n--------------YOU WON--------------\n\nSCORE: %d\n\n\n ('r' to restart, else to exit)\n\n\n", score);
    char want1 = getch();
    if (want1 == 'r')
    {
        goto start;
    }
    // SAVE
    FILE* fp1 = fopen("save.txt", "w");
    char fpBuffer1[100];
    sprintf(fpBuffer1, "%d", score);
    fprintf(fp1, fpBuffer1);
    fclose(fp1);
    return 0;
}