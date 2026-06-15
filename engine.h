#pragma once

#include <stdbool.h>

typedef struct {
  int x;
  int y;
} Point;

typedef enum {
  UP,
  LEFT,
  RIGHT,
  DOWN

} Rotation;

typedef struct {
  int nPoints;
  Point* points;
} Polyomino;


Point getPolyominoPoint(Polyomino polyomino, Point position, Rotation rotation, int i);

bool canAddToBoard(int w, int h, int* board, Point position, Polyomino polyomino, Rotation rotation);
void addToBoard(int w, int h, int* board, Point position, Polyomino polyomino, Rotation rotation, int id);
void removeFromBoard(int w, int h, int* board, Point position, Polyomino polyomino, Rotation rotation);


typedef struct {
  int width;
  int height;

  int nPolyominoTypes;
  Polyomino* polyominoTypes;
  int* values;
  int* available;

  int* penalties;
} Input;

typedef enum {
  PRESET_TETRIS,
  PRESET_SIMPLE,
  PRESET_RANDOM,
    PRESET_OBSTACLE,
    PRESET_IRREGULAR
} PresetType;

typedef enum {
  PENALTY_UNIFORM,
  PENALTY_BAD_DIAGONAL,
  PENALTY_GOOD_DIAGONAL,
  PENALTY_BAD_CORNERS,
  PENALTY_GOOD_CORNERS,
  PENALTY_CHECKERBOARD,
    PENALTY_OBSTACLE,
    PENALTY_MODULO
} PenaltyType;

Input createInput(int width, int height, PresetType preset, PenaltyType penalty);
void freeInput(Input input);


typedef struct {
  int* board;
  int* used;
  int score;
} State;

State createState(const Input* input);
void freeState(State* state);

bool canAddToState(const Input* input, const State* state, int polyominoIndex, Point position, Rotation rotation);
void addToState(const Input* input, State* state, int polyominoIndex, Point position, Rotation rotation);
void removeFromState(const Input* input, State* state, int polyominoIndex, Point position, Rotation rotation);
void toCsv(const Input* input, State* state, const char* path);


typedef struct {
  int polyominoIndex;
  Point point;
  Rotation rotation;
} Gen;


typedef struct {
  Gen **genes;
  int successful_mutations[6];
} Genotype;


Genotype* createRandomStartingState(Input* input, State* state);
bool alterOneGeneMutation(Input* input, State* state, Genotype* genotype);
bool removeOneGeneMutation(Input* input, State* state, Genotype* genotype);
bool addOneGeneMutation(Input* input, State* state, Genotype* genotype);
bool shiftOneGeneMutation(Input* input, State* state, Genotype* genotype);
bool rotateOneGeneMutation(Input* input, State* state, Genotype* genotype);
bool clearAreaMutation(Input* input, State* state, Genotype* genotype);
int mutate(Input* input, State* state, Genotype* genotype, double* weights, int num_weights);


Genotype* copyGenotype(const Input* input, const Genotype* src);

State copyState(const Input* input, const State* src);

Genotype* crossover(const Input* input, const Genotype* parentA, const Genotype* parentB);
State buildStateFromGenotype(const Input* input, Genotype* genotype);