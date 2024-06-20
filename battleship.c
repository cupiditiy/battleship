#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BOARD_SIZE 10
#define MAX_SHIP 5
#define A 65
#define SERIAL_TILE_LEN 3
#define SAVE_FILE "savedGame.bin"
#define TOP_SCORES_SAVED 10
#define SCORES_FILE "topTenScores.txt"

// Ship board
//{
typedef struct Ship {
  char name[25];
  int size;
  int shipNum;
} Ship;
typedef enum Orientation { Vertical, Horizontal } Orientation;
// Clamps the given value between the passed (inclusive) range.
int clampInt(int value, int min, int max) {
  if (value < min) {
    value = min;
  } else if (value > max) {
    value = max;
  }
  return value;
}

// Checks if a ship of a passed size can fit on the board at a given location in
// a given direction
int isValidLocation(int size, Ship ***board, int row, int col,
                    Orientation dir) {
  // Leftmost/topmost grid locations to sweep through
  int horCheckMin = clampInt(col - 1, 0, BOARD_SIZE - 1);
  int vertCheckMin = clampInt(row - 1, 0, BOARD_SIZE - 1);

  // Bottommost/rightmost grid locations to sweep through
  int horCheckMax, vertCheckMax;
  if (dir == Horizontal) {
    horCheckMax = clampInt(col + size, 0, BOARD_SIZE - 1);
    vertCheckMax = clampInt(row + 1, 0, BOARD_SIZE - 1);
  } else { // vertical
    horCheckMax = clampInt(col + 1, 0, BOARD_SIZE - 1);
    vertCheckMax = clampInt(row + size, 0, BOARD_SIZE - 1);
  }
  // Checks through all necessary grid locations to ensure no overlap/touching
  // with other ships
  for (int i = vertCheckMin; i <= vertCheckMax; i++) {
    for (int j = horCheckMin; j <= horCheckMax; j++) {
      if (board[i][j] != NULL) {
        return 0;
      }
    }
  }
  return 1;
}
// Places a ship on a ship board. Assumes that the position is valid.
void placeShip(Ship *ship, Ship ***board, int row, int col, Orientation dir) {
  int sizeOffset = ship->size - 1;
  int colMax = col + (dir * sizeOffset);
  int rowMax = row + (((dir + 1) % 2) * sizeOffset);
  for (int i = row; i <= rowMax; i++) {
    for (int j = col; j <= colMax; j++) {
      board[i][j] = ship;
    }
  }
}
// Randomly places ships on a ship board
void populateShipBoard(Ship ships[], Ship ***board, int shipCount) {
  for (int i = 0; i < shipCount; i++) {
    Ship *ship = &ships[i];
    Orientation dir = rand() % 2;
    int row = 0, col = 0, sizeOffset = ship->size - 1;
    do {
      // Not all rows are available as starting tile if ship is vertical
      row = rand() % (BOARD_SIZE - (((dir + 1) % 2) * sizeOffset));
      // Not all columns are available as starting tile if ship is horizontal
      col = rand() % (BOARD_SIZE - (dir * sizeOffset));

    } while (isValidLocation(ship->size, board, row, col, dir) == 0);
    placeShip(ship, board, row, col, dir);
  }
}
// Initializes an empty 2D array of Ship pointers
Ship ***initShipBoard(Ship ships[], int shipCount) {
  Ship ***board = malloc(BOARD_SIZE * sizeof(Ship **));
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[i] = malloc(BOARD_SIZE * sizeof(Ship *));
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = NULL;
    }
  }
  return board;
}
// Prints a board with all ship locations to console. For debugging purpose only
void debugPrintShipBoard(Ship ***board) {
  // Print top row of column letters
  printf("   ");
  for (int i = 0; i < BOARD_SIZE; i++) {
    printf("[%c]", (char)(A + i));
  }
  printf("\n");

  for (int i = 0; i < BOARD_SIZE; i++) {
    // Print side column of row numbers
    printf("[%d] ", i + 1);

    // Print row content
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] == NULL) {
        printf("   ");
      } else {
        printf("%c  ", board[i][j]->name[0]);
      }
    }
    printf("\n");
  }
}
//}

// Display board
//{
// Initializes an empty 2D char array
char **initDisplayBoard() {
  char **board = malloc(BOARD_SIZE * sizeof(char *));
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[i] = malloc(BOARD_SIZE * sizeof(char));
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = '\0';
    }
  }
  return board;
}
// Prints a game board to console
void printBoard(char **board) {
  // Print top row of column letters
  printf("    ");
  for (int i = 0; i < BOARD_SIZE; i++) {
    printf("[%c]", (char)(A + i));
  }
  printf("\n");

  for (int i = 0; i < BOARD_SIZE; i++) {
    // Print side column of row numbers
    if (i <= 10) {
      printf("[%d]", i + 1);
    }

    // Print row content
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] == '\0') {
        printf("   ");
      } else {
        printf(" %c ", board[i][j]);
      }
    }
    printf("\n");
  }
}
// Frees memory allocated to a 2D array
void freeBoard(void **board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    free(board[i]);
  }
  free(board);
}
//}

// Input/game logic
//{
// Parses user input as a board grid space
void getGameInput(int *col, int *row) {
  printf("Input a target. ");
  int isValidInput = 0;
  char input[4] = {'\0'};

  while (isValidInput == 0) {
    fflush(stdin);
    scanf("%3[^\n]s", input);

    // Cast char to ASCII then shift it so A is 0
    int colIn = (int)input[0] - A;
    // Cast char to int. atoi is 0 if input cannot be parsed to an int
    int rowIn = atoi(&input[1]);

    if (input[0] == 'Q') {
      *col = -1;
      return;
    }
    if (colIn < 0 || colIn >= BOARD_SIZE) {
      printf("Please select a valid column. \n");
    } else if (rowIn < 1 || rowIn > BOARD_SIZE) {
      printf("Please select a valid row. \n");
    } else {
      *col = colIn;
      // Subtract 1 so row can be used as an index
      *row = rowIn - 1;
      isValidInput = 1;
    }
  }
}
// Checks if a passed ship has sunk
int hasSunk(char **board, Ship ***shipBoard, Ship *ship) {
  char *hitTiles[MAX_SHIP];
  int hitCount = 0;
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      // Check if the tile is both occupied by the ship and has been hit by the
      // player
      if (shipBoard[i][j] == ship && board[i][j] != '\0') {
        hitTiles[hitCount] = &board[i][j];
        hitCount++;
      }
    }
  }
  if (hitCount >= ship->size) {
    // Set all tiles occupied by the sunk ship to that ship's letter
    for (int i = 0; i < hitCount; i++) {
      *hitTiles[i] = ship->name[0];
    }
    return 1;
  }
  return 0;
}
// Check if the game has been won (all ships sunk)
int hasWon(char **board, Ship ***shipBoard, Ship ships[], int shipCount) {
  for (int i = 0; i < shipCount; i++) {
    if (hasSunk(board, shipBoard, &ships[i]) == 0) {
      return 0;
    }
  }
  return 1;
}
// Checks and processes the target coordinate as a hit or miss.
void processHit(Ship ***ships, char **board, int tRow, int tCol, int *score) {
  char *tile = &board[tRow][tCol];
  Ship *ship = ships[tRow][tCol];

  if (*tile != '\0') {
    printf("You already fired there!\n");
  } else if (ship != NULL) {
    printf("It's a hit!\n");
    *tile = 'X';
    *score += 1;

    if (hasSunk(board, ships, ship)) {
      printf("Ship sunk!\n");
    }
  } else {
    printf("It's a miss!\n");
    *tile = 'O';
    *score += 1;
  }
  printf("CURRENT SCORE: %d\n", *score);
}
// Returns number of turns needed to produce specified game board
int getScore(char **board) {
  int score = 0;
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] != '\0') {
        score++;
      }
    }
  }
  return score;
}
//}

// Score saving/loading
//{
// Returns the number of digits in a number
int getDigits(int x) { return floor(log10(x)) + 1; }
// Sorts an array of numbers in a low-to-high order
int *sortAscending(int *arr, int size) {
  int temp;
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
  return arr;
}
// Reads data from a CSV file into an integer array
int *readSavedScores(char *filename) {
  // 3 for digits (100 max), 1 for comma
  int bufferLength =
      TOP_SCORES_SAVED * (getDigits(BOARD_SIZE * BOARD_SIZE) + 1);
  char buf[bufferLength];
  char delim[2] = ",";
  char *token;
  int count = 0;

  int *scores = malloc(TOP_SCORES_SAVED * sizeof(int));

  // Read and add each comma-separated value ("token") to array
  FILE *fp = fopen(filename, "r");
  fgets(buf, bufferLength, fp);
  token = strtok(buf, delim);
  while (token != NULL) {
    scores[count] = atoi(token);
    count++;
    token = strtok(NULL, delim);
  }
  fclose(fp);
  return scores;
}
// Writes an integer array as a comma-separated list to a file
void writeSavedScores(int *scores, char *filename) {
  char delim[2] = ",";
  char strScore[4] = {'\0'};

  FILE *fp = fopen(filename, "w");
  for (int i = 0; i < TOP_SCORES_SAVED; i++) {
    sprintf(strScore, "%d", scores[i]);
    fwrite(strScore, sizeof(char), getDigits(scores[i]), fp);
    // Write a comma for all but the last element
    if (i != TOP_SCORES_SAVED - 1) {
      fwrite(delim, sizeof(char), 1, fp);
    }
  }
  fclose(fp);
}
// Adds specified score to a saved scores file if specified score is within the
// top ten
void saveScore(int score, char *filename) {
  int *savedScores = sortAscending(readSavedScores(filename), TOP_SCORES_SAVED);
  // Replace new score only if it is lower than the highest score saved
  if (score < savedScores[TOP_SCORES_SAVED - 1]) {
    savedScores[TOP_SCORES_SAVED - 1] = score;
    sortAscending(savedScores, TOP_SCORES_SAVED);
    writeSavedScores(savedScores, filename);
  }
  free(savedScores);
}
//}

// Game saving
//{
// Writes data from display board to file
void serializeDisplayBoard(char **dispBoard, FILE *fp) {
  // Writes ASCII code of each character of board in 2 digit format
  char tileData[SERIAL_TILE_LEN] = {'\0'};
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      sprintf(tileData, "%02d", (int)dispBoard[i][j]);
      fwrite(tileData, sizeof(char), SERIAL_TILE_LEN - 1, fp);
    }
  }
}
// Writes data from ship board to file
void serializeShipBoard(Ship ***shipBoard, FILE *fp) {
  // Writes ship number of tile occupied by a ship on the board in 2 digit
  // format. Supports up to 99 ships
  char tileData[SERIAL_TILE_LEN] = {'\0'};
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (shipBoard[i][j] != NULL) {
        sprintf(tileData, "%02d", shipBoard[i][j]->shipNum);
      } else {
        strcpy(tileData, "00");
      }
      fwrite(tileData, sizeof(char), SERIAL_TILE_LEN - 1, fp);
    }
  }
}
// Serializes the state of a game in progress and saves it to a specified file
void saveState(char **dispBoard, Ship ***shipBoard, char *filename) {
  FILE *fp = fopen(filename, "wb");
  serializeDisplayBoard(dispBoard, fp);
  serializeShipBoard(shipBoard, fp);
  fclose(fp);
}
//}

// Game loading
//{
// Populates a 2D char array display board with saved data. Assumes that
// BOARD_SIZE has not changed since initial serialization
void deserializeDisplayBoard(char **dispBoard, FILE *fp) {
  char buffer[SERIAL_TILE_LEN] = {'\0'};
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      fread(buffer, sizeof(char), 2, fp);
      dispBoard[i][j] = (char)atoi(buffer);
    }
  }
}
// Loads ship position from saved game to 2D ship pointer array. Assumes that
// BOARD_SIZE has not changed since initial serialization.
void deserializeShipBoard(Ship ***shipBoard, Ship ships[], FILE *fp) {
  char buffer[SERIAL_TILE_LEN] = {'\0'};
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      fread(buffer, sizeof(char), 2, fp);
      int num = atoi(buffer);
      if (num > 0) {
        shipBoard[i][j] = &ships[atoi(buffer) - 1];
      }
    }
  }
}
// Deserializes stored file data to recreate game state. Returns 0 if no game
// state is saved
int loadState(char **dispBoard, Ship ***shipBoard, Ship ships[],
              char *filename) {
  FILE *fp;
  if ((fp = fopen(filename, "rb"))) {
    deserializeDisplayBoard(dispBoard, fp);
    deserializeShipBoard(shipBoard, ships, fp);
    return 1;
  }
  return 0;
}
//}

// Main
//{
void playGame(char **dispBoard, Ship ***shipBoard, Ship ships[],
              int shipCount) {
  int score = getScore(dispBoard);
  int col;
  int row;
  while (hasWon(dispBoard, shipBoard, ships, shipCount) == 0) {
    printBoard(dispBoard);
    printf("\n");
    // debugPrintShipBoard(shipBoard);

    getGameInput(&col, &row);

    // col will be -1 if user input is 'Q' for quit
    if (col == -1) {
      printf("Saving and quitting.\n\n");
      saveState(dispBoard, shipBoard, SAVE_FILE);
      return;
    }

    processHit(shipBoard, dispBoard, row, col, &score);
  }
  printf("Congratulations! You sunk all enemy ships and won the game!\n\n");
  saveScore(score, SCORES_FILE);
}

int main(void) {
  Ship ships[] = {{"Seminole State Ship", 3, 1},
                  {"Air Force Academy", 5, 2},
                  {"Valencia Destroyer", 4, 3},
                  {"Eskimo University", 3, 4},
                  {"DeLand High School", 2, 5}};
  int shipCount = sizeof(ships) / sizeof(Ship);

  printf("Welcome to Valencia Battleship.\n");
  srand(time(NULL));

  char input = '\0';
  while (input != 'D') {
    printf("A) Start a new game\n");
    printf("B) Load saved game\n");
    printf("C) See best scores\n");
    printf("D) Quit\n");

    fflush(stdin);
    scanf(" %c", &input);

    char **dispBoard = initDisplayBoard();
    Ship ***shipBoard = initShipBoard(ships, shipCount);
    switch (input) {
    case 'A': {
      populateShipBoard(ships, shipBoard, shipCount);
      playGame(dispBoard, shipBoard, ships, shipCount);
      debugPrintShipBoard(shipBoard);
      break;
    }
    case 'B': {
      if (loadState(dispBoard, shipBoard, ships, SAVE_FILE)) {
        playGame(dispBoard, shipBoard, ships, shipCount);
      } else {
        printf("No saved game found!\n");
      }
      break;
    }
    case 'C': {
      int *scores = readSavedScores(SCORES_FILE);
      printf("Top 10 scores:\n");
      for (int i = 0; i < TOP_SCORES_SAVED; i++) {
        if (scores[i] <= BOARD_SIZE * BOARD_SIZE) {
          printf("%d: %d\n", i + 1, scores[i]);
        }
      }
      free(scores);
      break;
    }
    }
    freeBoard((void **)dispBoard);
    freeBoard((void **)shipBoard);
  }
  return 0;
}
//}
