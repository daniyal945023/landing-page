#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

// Screen and Track Dimensions
#define SCREEN_WIDTH 50
#define SCREEN_HEIGHT 20
#define TRACK_LEFT 5
#define TRACK_RIGHT 45

// Lap tracking
#define LAP_THRESHOLD 10  // Number of obstacles to avoid for 1 lap

// Game State Variables
int carX, carY;
int obstacleX, obstacleY;
int score;
int gameOver;
int gameMode; // 0: Menu/Quit, 1: Endless Survival, 2: Time Trial
int totalLaps;
int obstaclesPassed;  // Track obstacles passed for lap calculation
time_t startTime, endTime;
double elapsedTime;

// File Handling Global Variables
int highScoreSurvival = 0;
double highScoreTimeTrial = 99999.0; // Initialize with a high value for lowest time
int totalGamesPlayed = 0;
int totalLapsCompleted = 0;


// --- Console Utility Functions ---

void gotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}


// --- File Handling Functions ---

void loadScores() {
    FILE *fptr = fopen("highscores.txt", "r");
    if (fptr != NULL) {
        fscanf(fptr, "Survival High Score: %d\n", &highScoreSurvival);
        fscanf(fptr, "Time Trial Best Time: %lf\n", &highScoreTimeTrial);
        fclose(fptr);
    }
}

void saveScores() {
    FILE *fptr = fopen("highscores.txt", "w");
    if (fptr != NULL) {
        fprintf(fptr, "Survival High Score: %d\n", highScoreSurvival);
        fprintf(fptr, "Time Trial Best Time: %.2lf\n", highScoreTimeTrial);
        fclose(fptr);
    }
}

void updateProgress() {
    FILE *fptr = fopen("player_progress.txt", "a"); // Append mode
    if (fptr != NULL) {
        totalGamesPlayed++;
        fprintf(fptr, "Game #%d played. Laps completed: %d. Score: %d.\n", totalGamesPlayed, totalLaps, score);
        fclose(fptr);
    }
}


// --- Game Logic Functions ---

void setupGame() {
    gameOver = 0;
    carX = SCREEN_WIDTH / 2;
    carY = SCREEN_HEIGHT - 2;
    obstacleX = rand() % (TRACK_RIGHT - TRACK_LEFT - 1) + TRACK_LEFT + 1;
    obstacleY = 0;
    score = 0;
    totalLaps = 0;
    obstaclesPassed = 0;
    elapsedTime = 0.0;
    time(&startTime); // Start the timer
    hideCursor();
}

void drawGame() {
    gotoXY(0, 0);
    int i, j;
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            if (j == TRACK_LEFT || j == TRACK_RIGHT) {
                printf("|");
            } else if (i == carY && j == carX) {
                printf("A");
            } else if (i == obstacleY && j == obstacleX) {
                printf("X");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("Score: %d | ", score);
    if (gameMode == 1) {
        printf("Mode: Survival | High Score: %d\n", highScoreSurvival);
    } else if (gameMode == 2) {
        printf("Mode: Time Trial | Laps: %d/3 | Time: %.2fs | Best Time: %.2fs\n", totalLaps, elapsedTime, highScoreTimeTrial);
    }
}

void inputGame() {
    if (_kbhit()) {
        switch (_getch()) {
            case 'a':
                if (carX > TRACK_LEFT + 1) carX--;
                break;
            case 'd':
                if (carX < TRACK_RIGHT - 1) carX++;
                break;
            case 'q':
                gameOver = 1;
                break;
        }
    }
}

void logicGame() {
    // Update elapsed time
    time(&endTime);
    elapsedTime = difftime(endTime, startTime);

    // Move obstacle
    obstacleY++;
    if (obstacleY >= SCREEN_HEIGHT) {
        obstacleY = 0;
        obstacleX = rand() % (TRACK_RIGHT - TRACK_LEFT - 1) + TRACK_LEFT + 1;
        score++;
        obstaclesPassed++;

        // Calculate laps based on obstacles passed
        if (gameMode == 2) {
            int newLaps = obstaclesPassed / LAP_THRESHOLD;
            if (newLaps > totalLaps) {
                totalLaps = newLaps;
            }
        }
    }

    // Collision detection
    if (carY == obstacleY && carX == obstacleX) {
        gameOver = 1;
    }
}

void gameLoop() {
    setupGame();
    while (!gameOver) {
        drawGame();
        inputGame();
        logicGame();

        if (gameMode == 2 && totalLaps >= 3) {
            gameOver = 1; // End game after 3 laps in time trial
            // Compare and update best time
            if (elapsedTime < highScoreTimeTrial) {
                highScoreTimeTrial = elapsedTime;
                saveScores();
            }
        }
        Sleep(40); // Control game speed
    }

    // Game Over Logic
    updateProgress();
    if (gameMode == 1 && score > highScoreSurvival) {
        highScoreSurvival = score;
        saveScores();
    }
    showCursor();
    gotoXY(0, SCREEN_HEIGHT + 4);
    printf("Game Over! Final Score: %d\n", score);
    if (gameMode == 2) {
        printf("Time: %.2f seconds\n", elapsedTime);
    }
    printf("Press 'm' to return to menu or 'q' to quit: ");
}


// --- Menu Functions ---

void displayMenu() {
    system("cls"); // Clear screen for menu
    printf("Console Racing Game\n");
    printf("-------------------\n");
    printf("High Scores:\n");
    printf("Survival: %d\n", highScoreSurvival);
    if (highScoreTimeTrial < 99999.0) {
        printf("Time Trial Best Time: %.2fs\n\n", highScoreTimeTrial);
    } else {
        printf("Time Trial Best Time: N/A\n\n");
    }
    printf("Select Game Mode:\n");
    printf("1. Endless Survival\n");
    printf("2. Time Trial (3 Laps)\n");
    printf("Q. Quit\n");
    printf("Enter choice: ");
}

int menuInput() {
    char choice;
    choice = _getch();
    switch (choice) {
        case '1':
            gameMode = 1; // Endless Survival
            gameLoop();
            return 1; // Keep running the menu loop after the game
        case '2':
            gameMode = 2; // Time Trial
            gameLoop();
            return 1; // Keep running the menu loop after the game
        case 'q':
        case 'Q':
            gameMode = 0; // Quit command
            return 0; // Signal main to stop the program
        case 'm': // In case user presses 'm' after game over
        case 'M':
            return 1; // Go back to menu display
        default:
            return 1; // Keep running the menu loop for invalid input
    }
}

int main() {
    srand(time(NULL));
    loadScores(); // Load scores at program start
    gameMode = 0; 
    int keepRunning = 1;

    while (keepRunning) {
        displayMenu();
        keepRunning = menuInput(); // menuInput now returns 0 to quit
    }

    printf("\nGoodbye!\n");
    return 0;
}