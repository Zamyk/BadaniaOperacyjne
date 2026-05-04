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

Input createSmallExampleInput();
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
} Genotype;


Genotype* createRandomStartingState(Input* input, State* state);
void alterOneGeneMutation(Input* input, State* state, Genotype* genotype);


Genotype* copyGenotype(const Input* input, const Genotype* src);

State copyState(const Input* input, const State* src);