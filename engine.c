#include <engine.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

// --- Polyomino ---

Point getPolyominoPoint(Polyomino polyomino, Point position, Rotation rotation, int i) {
    Point p = polyomino.points[i];
    Point rotated;

    switch (rotation) {
        case UP:    rotated = (Point){ p.x,  p.y}; break;
        case LEFT:  rotated = (Point){ p.y, -p.x}; break;
        case DOWN:  rotated = (Point){-p.x, -p.y}; break;
        case RIGHT: rotated = (Point){-p.y,  p.x}; break;
    }

    return (Point){ position.x + rotated.x, position.y + rotated.y };
}

// --- Board ---

bool canAddToBoard(int w, int h, int* board, Point position, Polyomino polyomino, Rotation rotation) {
    for (int i = 0; i < polyomino.nPoints; ++i) {
        Point p = getPolyominoPoint(polyomino, position, rotation, i);
        if (p.x < 0 || p.x >= w || p.y < 0 || p.y >= h) {
            return false;
        }
        if (board[p.x + w * p.y] != -1) {
            return false;
        }
    }
    return true;
}

void addToBoard(int w, int h, int* board, Point position, Polyomino polyomino, Rotation rotation, int id) {
    (void) h;
    for (int i = 0; i < polyomino.nPoints; ++i) {
        Point p = getPolyominoPoint(polyomino, position, rotation, i);
        board[p.x + w * p.y] = id;
    }
}

void removeFromBoard(int w, int h, int* board, Point position, Polyomino polyomino, Rotation rotation) {
    (void) h;
    for (int i = 0; i < polyomino.nPoints; i++) {
        Point p = getPolyominoPoint(polyomino, position, rotation, i);
        board[p.x + w * p.y] = -1;
    }
}

// --- Input ---

Input createSmallExampleInput() {
    Input input;
    input.width  = 10;
    input.height = 10;
    input.nPolyominoTypes = 3;

    input.polyominoTypes = malloc(3 * sizeof(Polyomino));

    // T-block
    //  X
    // XXX
    input.polyominoTypes[0].nPoints = 4;
    input.polyominoTypes[0].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[0].points[0] = (Point){0, 0};
    input.polyominoTypes[0].points[1] = (Point){1, 0};
    input.polyominoTypes[0].points[2] = (Point){2, 0};
    input.polyominoTypes[0].points[3] = (Point){1, 1};

    // L-block
    // X
    // X
    // XX
    input.polyominoTypes[1].nPoints = 4;
    input.polyominoTypes[1].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[1].points[0] = (Point){0, 0};
    input.polyominoTypes[1].points[1] = (Point){0, 1};
    input.polyominoTypes[1].points[2] = (Point){0, 2};
    input.polyominoTypes[1].points[3] = (Point){1, 2};

    // S-block
    //  XX
    // XX
    input.polyominoTypes[2].nPoints = 4;
    input.polyominoTypes[2].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[2].points[0] = (Point){0, 0};
    input.polyominoTypes[2].points[1] = (Point){1, 0};
    input.polyominoTypes[2].points[2] = (Point){1, 1};
    input.polyominoTypes[2].points[3] = (Point){2, 1};

    // values = 0 for all types
    input.values = calloc(3, sizeof(int));

    // available = 5 of each
    input.available = malloc(3 * sizeof(int));
    input.available[0] = 8;
    input.available[1] = 8;
    input.available[2] = 8;

    // penalties = 1 for each cell (10x10 = 100 cells)
    input.penalties = malloc(100 * sizeof(int));
    for (int i = 0; i < 100; i++) {
        input.penalties[i] = 1;
    }

    return input;
}

void freeInput(Input input) {
    for (int i = 0; i < input.nPolyominoTypes; i++) {
        free(input.polyominoTypes[i].points);
    }
    free(input.polyominoTypes);
    free(input.values);
    free(input.available);
    free(input.penalties);
}

// --- State ---

State createState(const Input* input) {
    State state;
    state.board = malloc(input->width * input->height * sizeof(int));
    for(int i = 0; i < input->width * input->height; i++) {
        state.board[i] = -1;
    }
    
    state.used = malloc(input->nPolyominoTypes * sizeof(int));
    state.score = 0;
    return state;
}

void freeState(State* state) {
    free(state->board);
    free(state->used);
}

bool canAddToState(const Input* input, const State* state, int polyominoIndex, Point position, Rotation rotation) {
    if(state->used[polyominoIndex] >= input->available[polyominoIndex]) {
        return false;
    }
    return canAddToBoard(input->width, input->height, state->board, position, input->polyominoTypes[polyominoIndex], rotation);
}

void addToState(const Input* input, State* state, int polyominoIndex, Point position, Rotation rotation) {
    addToBoard(input->width, input->height, state->board, position, input->polyominoTypes[polyominoIndex], rotation, polyominoIndex);
    state->used[polyominoIndex]++;
    state->score += input->values[polyominoIndex];

    for (int i = 0; i < input->polyominoTypes[polyominoIndex].nPoints; ++i) {
        Point p = getPolyominoPoint(input->polyominoTypes[polyominoIndex], position, rotation, i);
        state->score -= input->penalties[p.x + input->width * p.y];
    }
}

void removeFromState(const Input* input, State* state, int polyominoIndex, Point position, Rotation rotation) {
    removeFromBoard(input->width, input->height, state->board, position, input->polyominoTypes[polyominoIndex], rotation);
    state->used[polyominoIndex]--;
    state->score -= input->values[polyominoIndex];

    for (int i = 0; i < input->polyominoTypes[polyominoIndex].nPoints; ++i) {
        Point p = getPolyominoPoint(input->polyominoTypes[polyominoIndex], position, rotation, i);
        state->score += input->penalties[p.x + input->width * p.y];
    }
}

void toCsv(const Input* input, State* state, const char* path) {
    FILE* f = fopen(path, "w");
    if (f == NULL) {
        perror("Failed to open file for CSV export");
        return;
    }

    for (int y = 0; y < input->height; y++) {
        for (int x = 0; x < input->width; x++) {
            int value = state->board[y * input->width + x];
            fprintf(f, "%d", value);

            if (x < input->width - 1) {
                fprintf(f, ",");
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);
    printf("Board state exported to %s\n", path);
}


void createRandomStartingState(Input* input, State* state) {
    srand(time(NULL));
    for(int p=0; p<input->width * input->height * 4; p++) {
        int x = rand() % (input->width + 1);
        int y = rand() % (input->height + 1);
        for(int i = 0; i < input->nPolyominoTypes; i++) {
            Rotation rotation = rand() % 4;
            if(canAddToState(input, state, i, (Point){x, y}, rotation)) {
                addToState(input, state, i, (Point){x, y}, rotation);
                break;
            }
        }
    }
    for(int i = 0; i < input->nPolyominoTypes; i++) {
        for(int y = 0; y < input->height; y++) {
            for(int x = 0; x < input->width; x++) {
                for(Rotation rotation = UP; rotation <= DOWN; rotation++) {
                    if(canAddToState(input, state, i, (Point){x, y}, rotation)) {
                        addToState(input, state, i, (Point){x, y}, rotation);
                    }
                }
            }
        }
    }
}