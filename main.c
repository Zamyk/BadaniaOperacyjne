#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"

int main(void) {
    srand(time(NULL));

    Input input = createSmallExampleInput();

    Genotype **genotypes = malloc(100 * sizeof(Genotype *));
    State *states = malloc(100 * sizeof(State));

    for (int i = 0; i < 10; i++) {
        State base_state = createState(&input);
        genotypes[i] = createRandomStartingState(&input, &base_state);
        states[i] = copyState(&input, &base_state); 
        printf("Score: %d\n", states[i].score);
    }

    int n_generations = 100; // liczba pokoleń

    for (int k = 0; k < n_generations; k++) {
        
        // 3. Dla każdej z 10 instancji (z indeksów 0-9) tworzymy 9 mutacji (na indeksach 10-99)
        int child_idx = 10;
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 9; j++) {
                states[child_idx] = copyState(&input, &states[i]);
                genotypes[child_idx] = copyGenotype(&input, genotypes[i]);
                
                alterOneGeneMutation(&input, &states[child_idx], genotypes[child_idx]);
                child_idx++;
            }
        }

        // 4. Sortujemy całą setkę malejąco po score
        for (int i = 0; i < 100 - 1; i++) {
            for (int j = 0; j < 100 - i - 1; j++) {
                if (states[j].score < states[j + 1].score) {
                    State temp_state = states[j];
                    states[j] = states[j + 1];
                    states[j + 1] = temp_state;

                    Genotype *temp_geno = genotypes[j];
                    genotypes[j] = genotypes[j + 1];
                    genotypes[j + 1] = temp_geno;
                }
            }
        }

        for (int i = 10; i < 100; i++) {
            free(genotypes[i]->genes); 
            free(genotypes[i]);
        }

        printf("Generation %d | Best score: %d\n", k, states[0].score);
    }

    printf("Best score: %d\n", states[0].score);
    toCsv(&input, &states[0], "best_final.csv");

    for (int i = 0; i < 10; i++) {
        free(genotypes[i]->genes);
        free(genotypes[i]);
    }
    free(genotypes);
    free(states);

    return 0;
}