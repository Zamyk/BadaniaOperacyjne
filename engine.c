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

static void applyPenalty(Input* input, PenaltyType type) {
    int totalCells = input->width * input->height;
    input->penalties = malloc(totalCells * sizeof(int));

    for (int y = 0; y < input->height; y++) {
        for (int x = 0; x < input->width; x++) {
            int idx = x + input->width * y;
            int penalty = 1;

            switch (type) {
                case PENALTY_UNIFORM:
                    penalty = 1;
                    break;
                case PENALTY_BAD_DIAGONAL:
                    if (x == y || x == input->width - 1 - y) penalty = -10;
                    break;
                case PENALTY_GOOD_DIAGONAL:
                    if (x == y || x == input->width - 1 - y) penalty = 10;
                    break;
                case PENALTY_BAD_CORNERS:
                    if ((x == 0 || x == input->width - 1) && (y == 0 || y == input->height - 1)) penalty = -10;
                    break;
                case PENALTY_GOOD_CORNERS:
                    if ((x == 0 || x == input->width - 1) && (y == 0 || y == input->height - 1)) penalty = 10;
                    break;
                case PENALTY_CHECKERBOARD:
                    if ((x + y) % 2 == 0) penalty = 5;
                    else penalty = -5;
                    break;
                case PENALTY_OBSTACLE:
                    if (x >= input->width/2 - 2 && x <= input->width/2 + 1 &&
                        y >= input->height/2 - 2 && y <= input->height/2 + 1) {
                        penalty = -1000;
                    } else {
                        penalty = 10;
                    }
                    break;
                case PENALTY_MODULO:
                    penalty = -((x * 3 + y * 7) % 5) * 5;
                    break;
            }
            input->penalties[idx] = penalty;
        }
    }
}

static Input createTetrisPreset(int width, int height) {
    Input input;
    input.width  = width;
    input.height = height;
    input.nPolyominoTypes = 3;

    input.polyominoTypes = malloc(3 * sizeof(Polyomino));

    // T-block
    input.polyominoTypes[0].nPoints = 4;
    input.polyominoTypes[0].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[0].points[0] = (Point){0, 0};
    input.polyominoTypes[0].points[1] = (Point){1, 0};
    input.polyominoTypes[0].points[2] = (Point){2, 0};
    input.polyominoTypes[0].points[3] = (Point){1, 1};

    // L-block
    input.polyominoTypes[1].nPoints = 4;
    input.polyominoTypes[1].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[1].points[0] = (Point){0, 0};
    input.polyominoTypes[1].points[1] = (Point){0, 1};
    input.polyominoTypes[1].points[2] = (Point){0, 2};
    input.polyominoTypes[1].points[3] = (Point){1, 2};

    // S-block
    input.polyominoTypes[2].nPoints = 4;
    input.polyominoTypes[2].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[2].points[0] = (Point){0, 0};
    input.polyominoTypes[2].points[1] = (Point){1, 0};
    input.polyominoTypes[2].points[2] = (Point){1, 1};
    input.polyominoTypes[2].points[3] = (Point){2, 1};

    input.values = malloc(3 * sizeof(int));
    input.values[0] = 0;
    input.values[1] = 0;
    input.values[2] = 0;

    input.available = malloc(3 * sizeof(int));
    input.available[0] = 1000000;
    input.available[1] = 1000000;
    input.available[2] = 1000000;

    return input;
}

static Input createSimplePreset(int width, int height) {
    Input input;
    input.width  = width;
    input.height = height;
    input.nPolyominoTypes = 1;

    input.polyominoTypes = malloc(1 * sizeof(Polyomino));

    // 2x1 block (domino)
    input.polyominoTypes[0].nPoints = 2;
    input.polyominoTypes[0].points  = malloc(2 * sizeof(Point));
    input.polyominoTypes[0].points[0] = (Point){0, 0};
    input.polyominoTypes[0].points[1] = (Point){1, 0};

    input.values = malloc(1 * sizeof(int));
    input.values[0] = 0; 

    input.available = malloc(1 * sizeof(int));
    input.available[0] = 1000000;

    return input;
}

static Input createRandomPreset(int width, int height) {
    Input input;
    input.width  = width;
    input.height = height;
    input.nPolyominoTypes = 5;

    input.polyominoTypes = malloc(input.nPolyominoTypes * sizeof(Polyomino));
    input.values = malloc(input.nPolyominoTypes * sizeof(int));
    input.available = malloc(input.nPolyominoTypes * sizeof(int));

    for (int i = 0; i < input.nPolyominoTypes; i++) {
        int size = (rand() % 5) + 2; // size between 1 and 5
        input.polyominoTypes[i].nPoints = size;
        input.polyominoTypes[i].points = malloc(size * sizeof(Point));
        
        // Very basic random polyomino generation: walk randomly
        input.polyominoTypes[i].points[0] = (Point){0, 0};
        for (int p = 1; p < size; p++) {
            Point neighbor;
            bool valid = false;
            while (!valid) {
                // Pick an existing point
                int existing_idx = rand() % p;
                Point existing = input.polyominoTypes[i].points[existing_idx];
                
                // Pick a direction
                int dir = rand() % 4;
                int dx = (dir == 0) ? 1 : (dir == 1) ? -1 : 0;
                int dy = (dir == 2) ? 1 : (dir == 3) ? -1 : 0;
                
                neighbor = (Point){existing.x + dx, existing.y + dy};
                
                // Check if already exists
                valid = true;
                for (int check = 0; check < p; check++) {
                    if (input.polyominoTypes[i].points[check].x == neighbor.x && 
                        input.polyominoTypes[i].points[check].y == neighbor.y) {
                        valid = false;
                        break;
                    }
                }
            }
            input.polyominoTypes[i].points[p] = neighbor;
        }

        input.values[i] = 0;
        input.available[i] = 1000000;
    }

    return input;
}

static Input createObstaclePreset(int width, int height) {
    Input input;
    input.width  = width;
    input.height = height;
    input.nPolyominoTypes = 2;

    input.polyominoTypes = malloc(2 * sizeof(Polyomino));

    input.polyominoTypes[0].nPoints = 6;
    input.polyominoTypes[0].points  = malloc(6 * sizeof(Point));
    input.polyominoTypes[0].points[0] = (Point){0, 0};
    input.polyominoTypes[0].points[1] = (Point){1, 0};
    input.polyominoTypes[0].points[2] = (Point){0, 1};
    input.polyominoTypes[0].points[3] = (Point){1, 1};
    input.polyominoTypes[0].points[4] = (Point){0, 2};
    input.polyominoTypes[0].points[5] = (Point){1, 2};

    input.polyominoTypes[1].nPoints = 3;
    input.polyominoTypes[1].points  = malloc(3 * sizeof(Point));
    input.polyominoTypes[1].points[0] = (Point){0, 0};
    input.polyominoTypes[1].points[1] = (Point){1, 0};
    input.polyominoTypes[1].points[2] = (Point){2, 0};

    input.values = malloc(2 * sizeof(int));
    input.values[0] = 0;
    input.values[1] = 0;

    input.available = malloc(2 * sizeof(int));
    input.available[0] = 100000;
    input.available[1] = 100000;

    return input;
}

static Input createIrregularPreset(int width, int height) {
    Input input;
    input.width  = width;
    input.height = height;
    input.nPolyominoTypes = 3;

    input.polyominoTypes = malloc(3 * sizeof(Polyomino));

    input.polyominoTypes[0].nPoints = 5;
    input.polyominoTypes[0].points  = malloc(5 * sizeof(Point));
    input.polyominoTypes[0].points[0] = (Point){0, 0};
    input.polyominoTypes[0].points[1] = (Point){1, 0};
    input.polyominoTypes[0].points[2] = (Point){2, 0};
    input.polyominoTypes[0].points[3] = (Point){0, 1};
    input.polyominoTypes[0].points[4] = (Point){2, 1};

    input.polyominoTypes[1].nPoints = 5;
    input.polyominoTypes[1].points  = malloc(5 * sizeof(Point));
    input.polyominoTypes[1].points[0] = (Point){1, 0};
    input.polyominoTypes[1].points[1] = (Point){0, 1};
    input.polyominoTypes[1].points[2] = (Point){1, 1};
    input.polyominoTypes[1].points[3] = (Point){2, 1};
    input.polyominoTypes[1].points[4] = (Point){1, 2};

    input.polyominoTypes[2].nPoints = 4;
    input.polyominoTypes[2].points  = malloc(4 * sizeof(Point));
    input.polyominoTypes[2].points[0] = (Point){0, 0};
    input.polyominoTypes[2].points[1] = (Point){0, 1};
    input.polyominoTypes[2].points[2] = (Point){0, 2};
    input.polyominoTypes[2].points[3] = (Point){1, 2};

    input.values = malloc(3 * sizeof(int));
    input.values[0] = 0;
    input.values[1] = 0;
    input.values[2] = 0;

    input.available = malloc(3 * sizeof(int));
    input.available[0] = 100000;
    input.available[1] = 100000;
    input.available[2] = 100000;

    return input;
}

Input createInput(int width, int height, PresetType preset, PenaltyType penalty) {
    Input input;
    switch(preset) {
        case PRESET_TETRIS:
            input = createTetrisPreset(width, height);
            break;
        case PRESET_SIMPLE:
            input = createSimplePreset(width, height);
            break;
        case PRESET_RANDOM:
            input = createRandomPreset(width, height);
            break;
        case PRESET_OBSTACLE:
            input = createObstaclePreset(width, height);
            break;
        case PRESET_IRREGULAR:
            input = createIrregularPreset(width, height);
            break;
    }
    applyPenalty(&input, penalty);
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
    
    state.used = calloc(input->nPolyominoTypes, sizeof(int));
    state.score = 0;
    for (int i = 0; i < input->width * input->height; i++) {
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
    Genotype *genotype = calloc(1, sizeof(Genotype));
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


bool alterOneGeneMutation(Input* input, State* state, Genotype* genotype) {
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

    if (count == 0) return false;

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
    return true;
}

bool removeOneGeneMutation(Input* input, State* state, Genotype* genotype) {
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
        return false;
    }

    int randomIndex = occupiedIndices[rand() % count];
    Gen* gen = genotype->genes[randomIndex];

    removeFromState(input, state, gen->polyominoIndex, gen->point, gen->rotation);

    free(gen);
    genotype->genes[randomIndex] = NULL;
    free(occupiedIndices);
    return true;
}

bool addOneGeneMutation(Input* input, State* state, Genotype* genotype) {
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
            return true;
        }
    }
    return false;
}

bool shiftOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int totalCells = input->width * input->height;
    int* occupiedIndices = malloc(totalCells * sizeof(int));
    int count = 0;
    for (int i = 0; i < totalCells; i++) {
        if (genotype->genes[i] != NULL) occupiedIndices[count++] = i;
    }
    if (count == 0) {
        free(occupiedIndices);
        return false;
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
        free(occupiedIndices);
        return true;
    } else {
        // revert
        addToState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
        genotype->genes[randomIndex] = gen;
        free(occupiedIndices);
        return false;
    }
}

bool rotateOneGeneMutation(Input* input, State* state, Genotype* genotype) {
    int totalCells = input->width * input->height;
    int* occupiedIndices = malloc(totalCells * sizeof(int));
    int count = 0;
    for (int i = 0; i < totalCells; i++) {
        if (genotype->genes[i] != NULL) occupiedIndices[count++] = i;
    }
    if (count == 0) {
        free(occupiedIndices);
        return false;
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
        free(occupiedIndices);
        return true;
    } else {
        // revert
        addToState(input, state, gen->polyominoIndex, gen->point, gen->rotation);
        genotype->genes[randomIndex] = gen;
        free(occupiedIndices);
        return false;
    }
}

bool clearAreaMutation(Input* input, State* state, Genotype* genotype) {
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
    return true;
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

State buildStateFromGenotype(const Input* input, Genotype* genotype) {
    State state = createState(input); 
    int n = input->width * input->height; 

    for (int i = 0; i < n; i++) {
        Gen* g = genotype->genes[i];
        if (g != NULL) { 
            // 1. Sprawdzamy, czy CAŁY klocek zmieści się na planszy bez kolizji
            if (canAddToState(input, &state, g->polyominoIndex, g->point, g->rotation)) {
                
                // 2. STAWIANIE KLOCKA: Zamiast g->polyominoIndex, przekazujemy 'i' (unikalne ID).
                // Dzięki temu każdy klocek otrzyma swój własny, unikalny kolor w Pythonie!
                addToBoard(input->width, input->height, state.board, g->point, input->polyominoTypes[g->polyominoIndex], g->rotation, i);
                
                // Aktualizacja statystyk stanu
                state.used[g->polyominoIndex]++;
                state.score += input->values[g->polyominoIndex];
                for (int p_idx = 0; p_idx < input->polyominoTypes[g->polyominoIndex].nPoints; ++p_idx) {
                    Point p = getPolyominoPoint(input->polyominoTypes[g->polyominoIndex], g->point, g->rotation, p_idx);
                    state.score += input->penalties[p.x + input->width * p.y];
                }

            } else {
                // 3. RESTRYKCJA: Jeśli wykryto nakładanie się, usuwamy wadliwy gen ewolucyjny
                free(genotype->genes[i]);
                genotype->genes[i] = NULL;
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