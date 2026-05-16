#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "engine.h"

int main(void) {
    srand(time(NULL));

    Input input = createSmallExampleInput();

    for(int x = 0; x < input.height && x < input.width; x++) {
        input.penalties[x + x * input.width] = -1000;
    }

    Genotype **genotypes = malloc(100 * sizeof(Genotype *));
    State *states = malloc(100 * sizeof(State));

    for (int i = 0; i < 10; i++) {
        State base_state = createState(&input);
        genotypes[i] = createRandomStartingState(&input, &base_state);
        states[i] = copyState(&input, &base_state); 
        printf("Score: %d\n", states[i].score);
    }


    int n_generations = 100; // liczba pokoleń
    int n_mutations = 6;
    double weights[6] = {10.0, 10.0, 10.0, 10.0, 10.0, 10.0};
    double crossover_probability = 0.7;

    for (int i = 0; i < 10 - 1; i++) {
        for (int j = 0; j < 10 - i - 1; j++) {
            if (states[j].score < states[j + 1].score) {
                State temp_s = states[j]; states[j] = states[j + 1]; states[j + 1] = temp_s;
                Genotype *temp_g = genotypes[j]; genotypes[j] = genotypes[j + 1]; genotypes[j + 1] = temp_g;
            }
        }
    }
    toCsv(&input, &states[0], "initial_best.csv"); 

    for (int k = 0; k < n_generations; k++) {
        
        // 3. Dla każdej z 10 instancji (z indeksów 0-9) tworzymy 9 mutacji (na indeksach 10-99)
        int child_idx = 10;
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 9; j++) {

                double r = (double)rand() / RAND_MAX;
                if (r > crossover_probability) {
                    // tylko mutacja
                    genotypes[child_idx] = copyGenotype(&input, genotypes[i]);
                } else {
                    // krzyżowanie + mutacja
                    int partner_idx = rand() % 10; 
                    genotypes[child_idx] = crossover(&input, genotypes[i], genotypes[partner_idx]);
                    // budujemy stan od zera, bo geny od dwóch rodziców mogą na siebie nachodzić
                    states[child_idx] = buildStateFromGenotype(&input, genotypes[child_idx]);
                }
                states[child_idx] = buildStateFromGenotype(&input, genotypes[child_idx]);
                int chosenMut = mutate(&input, &states[child_idx], genotypes[child_idx], weights, n_mutations);
                freeState(&states[child_idx]);
                states[child_idx] = buildStateFromGenotype(&input, genotypes[child_idx]);

                int parentScore = states[i].score;
                
                if (states[child_idx].score > parentScore) {
                    weights[chosenMut] += 1.0;
                }
                
                child_idx++;
            }
        }
        
        for (int m = 0; m < n_mutations; m++) {
            weights[m] *= 0.99;
            if (weights[m] < 1.0) weights[m] = 1.0;
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