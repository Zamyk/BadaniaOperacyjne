#include <stdio.h>
#include "engine.h"

// bool tryAdd(Input* input, State* state) {
//     for(int i = 0; i < input->nPolyominoTypes; i++) {
//         for(int y = 0; y < input->height; y++) {
//             for(int x = 0; x < input->width; x++) {
//                 for(Rotation rotation = UP; rotation <= DOWN; rotation++) {
//                     if(canAddToState(input, state, i, (Point){x, y}, rotation)) {
//                         addToState(input, state, i, (Point){x, y}, rotation);
//                         return true;
//                     }
//                 }
//             }
//         }
//     }
//     return false;
// }

int main(void) {
    Input input = createSmallExampleInput();
    State state = createState(&input);

    // while(tryAdd(&input, &state)) {

    // }

    createRandomStartingState(&input, &state);

    toCsv(&input, &state, "out.csv");
}