#include "engine.h"

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
    // testowałem wartości dla klocków
    // input.values = malloc(3 * sizeof(int));
    // input.values[0] = 10; // T-block wart 10
    // input.values[1] = 10; // L-block wart 10
    // input.values[2] = 10; // S-block wart 10

    // available = 5 of each
    input.available = malloc(3 * sizeof(int));
    input.available[0] = 80;
    input.available[1] = 80;
    input.available[2] = 80;

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
    for (int i = 0; i < 100; i++) {
        state.score -= input->penalties[i]; 
    }
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
        state->score += input->penalties[p.x + input->width * p.y];
    }
}

void removeFromState(const Input* input, State* state, int polyominoIndex, Point position, Rotation rotation) {
    removeFromBoard(input->width, input->height, state->board, position, input->polyominoTypes[polyominoIndex], rotation);
    state->used[polyominoIndex]--;
    state->score -= input->values[polyominoIndex];

    for (int i = 0; i < input->polyominoTypes[polyominoIndex].nPoints; ++i) {
        Point p = getPolyominoPoint(input->polyominoTypes[polyominoIndex], position, rotation, i);
        state->score -= input->penalties[p.x + input->width * p.y];
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
    //printf("Board state exported to %s\n", path);
}


Genotype* createRandomStartingState(Input* input, State* state) {
    //srand(3);
    Genotype *genotype = malloc(sizeof(Genotype));
    genotype->genes = malloc(input->width * input->height * sizeof(Gen *));

    for(int i = 0; i < input->width * input->height; i++) {
        genotype->genes[i] = NULL;
    }

    for(int p=0; p<input->width * input->height * 4; p++) {
        int x = rand() % (input->width);
        int y = rand() % (input->height);
        for(int i = 0; i < input->nPolyominoTypes; i++) {
            Rotation rotation = rand() % 4;
            if(canAddToState(input, state, i, (Point){x, y}, rotation)) {
                addToState(input, state, i, (Point){x, y}, rotation);
                Gen* newGen = malloc(sizeof(Gen));
                newGen->polyominoIndex = i;
                newGen->point = (Point){x, y};
                newGen->rotation = rotation;
                
                genotype->genes[x + input->width * y] = newGen;
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
                        Gen* newGen = malloc(sizeof(Gen));
                        newGen->polyominoIndex = i;
                        newGen->point = (Point){x, y};
                        newGen->rotation = rotation;
                        
                        genotype->genes[x + input->width * y] = newGen;
                    }
                }
            }
        }
    }

    return genotype;
}


void alterOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int width = input->width;
    int height = input->height;
    int totalCells = width * height;

    int occupiedIndices[totalCells];
    int count = 0;

    for (int i = 0; i < totalCells; i++) {
        if (genotype->genes[i] != NULL) {
            occupiedIndices[count++] = i;
        }
    }

    if (count == 0) return;

    int randomIndex = occupiedIndices[rand() % count];
    Gen* gen = genotype->genes[randomIndex];

    removeFromState(input, state, gen->polyominoIndex, gen->point, gen->rotation);

    free(gen); 
    genotype->genes[randomIndex] = NULL;

    for(int i = 0; i < input->nPolyominoTypes; i++) {
        for(int y = 0; y < input->height; y++) {
            for(int x = 0; x < input->width; x++) {
                for(Rotation rotation = UP; rotation <= DOWN; rotation++) {
                    if(canAddToState(input, state, i, (Point){x, y}, rotation)) {
                        addToState(input, state, i, (Point){x, y}, rotation);
                        Gen* newGen = malloc(sizeof(Gen));
                        newGen->polyominoIndex = i;
                        newGen->point = (Point){x, y};
                        newGen->rotation = rotation;
                        
                        genotype->genes[x + input->width * y] = newGen;
                    }
                }
            }
        }
    }
}

void removeOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int totalCells = input->width * input->height;
    int* occupiedIndices = malloc(totalCells * sizeof(int));
    int count = 0;

    for (int i = 0; i < totalCells; i++) {
        if (genotype->genes[i] != NULL) {
            occupiedIndices[count++] = i;
        }
    }

    if (count == 0) {
        free(occupiedIndices);
        return;
    }

    int randomIndex = occupiedIndices[rand() % count];
    Gen* gen = genotype->genes[randomIndex];

    removeFromState(input, state, gen->polyominoIndex, gen->point, gen->rotation);

    free(gen);
    genotype->genes[randomIndex] = NULL;
    free(occupiedIndices);
}

void addOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int maxAttempts = 100;
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        int x = rand() % input->width;
        int y = rand() % input->height;
        int polyType = rand() % input->nPolyominoTypes;
        Rotation rot = rand() % 4;

        if (canAddToState(input, state, polyType, (Point){x, y}, rot)) {
            addToState(input, state, polyType, (Point){x, y}, rot);
            Gen* newGen = malloc(sizeof(Gen));
            newGen->polyominoIndex = polyType;
            newGen->point = (Point){x, y};
            newGen->rotation = rot;
            genotype->genes[x + input->width * y] = newGen;
            return;
        }
    }
}

void shiftOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int totalCells = input->width * input->height;
    int* occupiedIndices = malloc(totalCells * sizeof(int));
    int count = 0;
    for (int i = 0; i < totalCells; i++) {
        if (genotype->genes[i] != NULL) occupiedIndices[count++] = i;
    }
    if (count == 0) {
        free(occupiedIndices);
        return;
    }
    int randomIndex = occupiedIndices[rand() % count];
    Gen* gen = genotype->genes[randomIndex];
    
    removeFromState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
    genotype->genes[randomIndex] = NULL;
    
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    int dir = rand() % 4;
    Point newPoint = {gen->point.x + dx[dir], gen->point.y + dy[dir]};
    
    if (canAddToState(input, state, gen->polyominoIndex, newPoint, gen->rotation)) {
        addToState(input, state, gen->polyominoIndex, newPoint, gen->rotation);
        gen->point = newPoint;
        genotype->genes[newPoint.x + input->width * newPoint.y] = gen;
    } else {
        // revert
        addToState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
        genotype->genes[randomIndex] = gen;
    }
    free(occupiedIndices);
}

void rotateOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int totalCells = input->width * input->height;
    int* occupiedIndices = malloc(totalCells * sizeof(int));
    int count = 0;
    for (int i = 0; i < totalCells; i++) {
        if (genotype->genes[i] != NULL) occupiedIndices[count++] = i;
    }
    if (count == 0) {
        free(occupiedIndices);
        return;
    }
    int randomIndex = occupiedIndices[rand() % count];
    Gen* gen = genotype->genes[randomIndex];
    
    removeFromState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
    genotype->genes[randomIndex] = NULL;
    
    Rotation newRot = (gen->rotation + 1 + (rand() % 3)) % 4; // pick any of the other 3
    
    if (canAddToState(input, state, gen->polyominoIndex, gen->point, newRot)) {
        addToState(input, state, gen->polyominoIndex, gen->point, newRot);
        gen->rotation = newRot;
        genotype->genes[randomIndex] = gen;
    } else {
        // revert
        addToState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
        genotype->genes[randomIndex] = gen;
    }
    free(occupiedIndices);
}

void clearAreaMutation(Input* input, State* state, Genotype* genotype) {
    int cx = rand() % input->width;
    int cy = rand() % input->height;
    int areaSize = 4;
    
    int minX = cx - areaSize / 2;
    int maxX = cx + areaSize / 2;
    int minY = cy - areaSize / 2;
    int maxY = cy + areaSize / 2;
    
    for (int i = 0; i < input->width * input->height; i++) {
        Gen* gen = genotype->genes[i];
        if (gen != NULL) {
            if (gen->point.x >= minX && gen->point.x <= maxX &&
                gen->point.y >= minY && gen->point.y <= maxY) {
                removeFromState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
                free(gen);
                genotype->genes[i] = NULL;
            }
        }
    }
}

Genotype* crossover(const Input* input, const Genotype* parentA, const Genotype* parentB) {
    Genotype* child = malloc(sizeof(Genotype));
    int n = input->width * input->height;
    
    child->genes = malloc(n * sizeof(Gen*));

    int cut = rand() % n;

    for (int i = 0; i < n; i++) {
        Gen* src = (i < cut) ? parentA->genes[i] : parentB->genes[i];
        
        if (src != NULL) {
            child->genes[i] = malloc(sizeof(Gen));
            child->genes[i]->polyominoIndex = src->polyominoIndex;
            child->genes[i]->point = src->point;
            child->genes[i]->rotation = src->rotation;
        } else {
            child->genes[i] = NULL;
        }
    }
    return child;
}

State buildStateFromGenotype(const Input* input, const Genotype* genotype) {
    State state = createState(input); 
    int n = input->width * input->height; 

    for (int i = 0; i < n; i++) {
        Gen* g = genotype->genes[i];
        if (g != NULL) { 
            if (canAddToState(input, &state, g->polyominoIndex, g->point, g->rotation)) {
                addToState(input, &state, g->polyominoIndex, g->point, g->rotation);
            }
        }
    }
    return state;
}

int mutate(Input* input, State* state, Genotype* genotype, double* weights, int num_weights) {
    double totalWeight = 0;
    for (int i = 0; i < num_weights; i++) {
        totalWeight += weights[i];
    }
    
    double r = ((double)rand() / (double)RAND_MAX) * totalWeight;
    double current = 0;
    int selected = num_weights - 1;
    for (int i = 0; i < num_weights; i++) {
        current += weights[i];
        if (r <= current) {
            selected = i;
            break;
        }
    }
    
    switch (selected) {
        case 0: alterOneGeneMutation(input, state, genotype); break;
        case 1: removeOneGeneMutation(input, state, genotype); break;
        case 2: addOneGeneMutation(input, state, genotype); break;
        case 3: shiftOneGeneMutation(input, state, genotype); break;
        case 4: rotateOneGeneMutation(input, state, genotype); break;
        case 5: clearAreaMutation(input, state, genotype); break;
        default: alterOneGeneMutation(input, state, genotype); selected = 0; break;
    }
    
    return selected;
}

State copyState(const Input* input, const State* src) {
    State dest;
    dest.score = src->score;

    int boardCells = input->width * input->height;
    int nTypes = input->nPolyominoTypes;

    dest.board = (int*)malloc(boardCells * sizeof(int));
    if (dest.board) {
        memcpy(dest.board, src->board, boardCells * sizeof(int));
    }

    dest.used = (int*)malloc(nTypes * sizeof(int));
    if (dest.used) {
        memcpy(dest.used, src->used, nTypes * sizeof(int));
    }

    return dest;
}


Genotype* copyGenotype(const Input* input, const Genotype* src) {
    if (src == NULL) return NULL;

    Genotype* dest = (Genotype*)malloc(sizeof(Genotype));

    dest->genes = (Gen**)malloc(input->width*input->height * sizeof(Gen*));
    
    for (int i = 0; i < input->width*input->height; i++) {
        if (src->genes[i] != NULL) {
            dest->genes[i] = (Gen*)malloc(sizeof(Gen));
            *(dest->genes[i]) = *(src->genes[i]);
        } else {
            dest->genes[i] = NULL;
        }
    }

    return dest;
}