#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "engine.h"

typedef struct {
    Genotype *genotype;
    State state;
    int successful_mutations[6];
    int failed_mutations[6];
} Entity;

typedef struct {
    int alterOneGeneMutation;
    int removeOneGeneMutation;
    int addOneGeneMutation;
    int shiftOneGeneMutation;
    int rotateOneGeneMutation;
    int clearAreaMutation;
} Params;

const char* MUTATION_NAMES[6] = {
    "alterOneGeneMutation",
    "removeOneGeneMutation",
    "addOneGeneMutation",
    "shiftOneGeneMutation",
    "rotateOneGeneMutation",
    "clearAreaMutation"
};

// Nowy format zapisu uwzględniający definicje polimino, pozycje oraz rotacje
void customToCsv(Input *input, Genotype *genotype, const char *filename) {
    if (!input || !genotype || !filename) {
        printf("[CSV WARNING] Invalid pointers passed to customToCsv.\n");
        return;
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Error opening file %s for writing.\n", filename);
        return;
    }

    // 1. Sekcja METADATA
    fprintf(f, "[METADATA]\n");
    fprintf(f, "width,%d\n", input->width);
    fprintf(f, "height,%d\n", input->height);

    // 2. Sekcja POLYOMINO_TYPES
    fprintf(f, "[POLYOMINO_TYPES]\n");
    if (input->polyominoTypes != NULL) {
        for (int t = 0; t < input->nPolyominoTypes; t++) {
            fprintf(f, "%d,%d", t, input->polyominoTypes[t].nPoints);
            if (input->polyominoTypes[t].points != NULL) {
                for (int p = 0; p < input->polyominoTypes[t].nPoints; p++) {
                    fprintf(f, ",%d,%d", input->polyominoTypes[t].points[p].x, input->polyominoTypes[t].points[p].y);
                }
            }
            fprintf(f, "\n");
        }
    }

    // 3. Sekcja PENALTIES
    fprintf(f, "[PENALTIES]\n");
    if (input->penalties != NULL) {
        for (int y = 0; y < input->height; y++) {
            for (int x = 0; x < input->width; x++) {
                fprintf(f, "%d", input->penalties[y * input->width + x]);
                if (x < input->width - 1) fprintf(f, ",");
            }
            fprintf(f, "\n");
        }
    }

    // 4. Sekcja PLACED_POLYOMINOES
    fprintf(f, "[PLACED_POLYOMINOES]\n");
    
    int total_cells = input->width * input->height;
    int unique_id_counter = 0;

    if (genotype->genes != NULL) {
        for (int i = 0; i < total_cells; i++) {
            // Bezpieczne pominięcie pustych slotów genu
            if (genotype->genes[i] == NULL) {
                continue;
            }

            int type_id = genotype->genes[i]->polyominoIndex;
            
            // Walidacja poprawności indeksu typu klocka
            if (type_id < 0 || type_id >= input->nPolyominoTypes) {
                continue;
            }

            int origin_x = genotype->genes[i]->point.x;
            int origin_y = genotype->genes[i]->point.y;
            int rotation_enum_val = (int)genotype->genes[i]->rotation; 

            fprintf(f, "%d,%d,%d,%d,%d\n", unique_id_counter, type_id, origin_x, origin_y, rotation_enum_val);
            unique_id_counter++;
        }
    }

    fclose(f);
}

void experiment(int starting_states, int single_state_duplications, int max_iterations, int mutations_per_iteration, int patience, Params params) {
    Input input = createSmallExampleInput();

    int total_weight = params.alterOneGeneMutation +
                       params.removeOneGeneMutation +
                       params.addOneGeneMutation +
                       params.shiftOneGeneMutation +
                       params.rotateOneGeneMutation +
                       params.clearAreaMutation;

    // Global counters for all mutation attempts
    int global_success[6] = {0, 0, 0, 0, 0, 0};
    int global_fail[6] = {0, 0, 0, 0, 0, 0};

    // Allocation of the parent population
    Entity *population = malloc(starting_states * sizeof(Entity));

    for (int i = 0; i < starting_states; i++) {
        State base_state = createState(&input);
        population[i].genotype = createRandomStartingState(&input, &base_state);
        population[i].state = buildStateFromGenotype(&input, population[i].genotype);
        freeState(&base_state);

        // Clear mutation history for the initial population
        for (int m = 0; m < 6; m++) {
            population[i].successful_mutations[m] = 0;
            population[i].failed_mutations[m] = 0;
        }
    }

    for (int i = 0; i < starting_states; i++) {
        char csv_name[64];
        sprintf(csv_name, "experiment_iter%d_rank%d.csv", 999999, i);
        customToCsv(&input, population[i].genotype, csv_name);
    }

    int best_score = -999999;
    int patience_counter = 0;

    for (int iter = 0; iter < max_iterations; iter++) {
        int total_children = starting_states * single_state_duplications;
        Entity *children = malloc(total_children * sizeof(Entity));

        int child_idx = 0;
        for (int i = 0; i < starting_states; i++) {
            for (int d = 0; d < single_state_duplications; d++) {
                
                // Klonowanie genotypu
                children[child_idx].genotype = copyGenotype(&input, population[i].genotype);

                // POPRAWKA 1: Dziecko zaczyna z zerowym licznikiem mutacji dla TEJ generacji
                for (int m = 0; m < 6; m++) {
                    children[child_idx].successful_mutations[m] = population[i].successful_mutations[m];
                    children[child_idx].failed_mutations[m] = population[i].failed_mutations[m];
                }

                // Najpierw budujemy stan początkowy dla dziecka
                children[child_idx].state = buildStateFromGenotype(&input, children[child_idx].genotype);

                // Mutujemy dziecko określone wielokrotnie
                for (int m = 0; m < mutations_per_iteration; m++) {
                    int r = rand() % total_weight;
                    int current_sum = 0;
                    bool success = false;
                    int chosen_mut = 0;

                    if (r < (current_sum += params.alterOneGeneMutation)) {
                        success = alterOneGeneMutation(&input, &children[child_idx].state, children[child_idx].genotype);
                        chosen_mut = 0;
                    } 
                    else if (r < (current_sum += params.removeOneGeneMutation)) {
                        success = removeOneGeneMutation(&input, &children[child_idx].state, children[child_idx].genotype);
                        chosen_mut = 1;
                    } 
                    else if (r < (current_sum += params.addOneGeneMutation)) {
                        success = addOneGeneMutation(&input, &children[child_idx].state, children[child_idx].genotype);
                        chosen_mut = 2;
                    } 
                    else if (r < (current_sum += params.shiftOneGeneMutation)) {
                        success = shiftOneGeneMutation(&input, &children[child_idx].state, children[child_idx].genotype);
                        chosen_mut = 3;
                    } 
                    else if (r < (current_sum += params.rotateOneGeneMutation)) {
                        success = rotateOneGeneMutation(&input, &children[child_idx].state, children[child_idx].genotype);
                        chosen_mut = 4;
                    } 
                    else {
                        success = clearAreaMutation(&input, &children[child_idx].state, children[child_idx].genotype);
                        chosen_mut = 5;
                    }

                    // Collect mutation statistics
                    if (success) {
                        global_success[chosen_mut]++;
                        children[child_idx].successful_mutations[chosen_mut]++;
                    } else {
                        global_fail[chosen_mut]++;
                        children[child_idx].failed_mutations[chosen_mut]++;
                    }
                    
                    // POPRAWKA 2: USUNIĘTO stąd freeState i buildStateFromGenotype!
                }

                // POPRAWKA 3: Budujemy ostateczny stan i wyliczamy score RAZ po wszystkich mutacjach
                freeState(&children[child_idx].state);
                children[child_idx].state = buildStateFromGenotype(&input, children[child_idx].genotype);

                child_idx++;
            }
        }

        // Combine parents and children into a single pool
        int pool_size = starting_states + total_children;
        Entity *pool = malloc(pool_size * sizeof(Entity));

        for (int i = 0; i < starting_states; i++) pool[i] = population[i];
        for (int i = 0; i < total_children; i++) pool[starting_states + i] = children[i];

        // Bubble sort pool descending by score
        for (int i = 0; i < starting_states; i++) {
            for (int j = 0; j < pool_size - i - 1; j++) {
                if (pool[j].state.score < pool[j + 1].state.score) {
                    Entity temp = pool[j];
                    pool[j] = pool[j + 1];
                    pool[j + 1] = temp;
                }
            }
        }

        // 1. Tworzymy tymczasową tablicę na nową populację
        Entity *new_population = malloc(starting_states * sizeof(Entity));

        for (int i = 0; i < starting_states; i++) {
            // Robimy pełną, głęboką kopię przetrwałych osobników
            new_population[i].genotype = copyGenotype(&input, pool[i].genotype);
            new_population[i].state = buildStateFromGenotype(&input, new_population[i].genotype);
            new_population[i].state.score = pool[i].state.score; // Kopiujemy też wynik, jeśli jest potrzebny

            // Kopiujemy tablice mutacji
            for (int m = 0; m < 6; m++) {
                new_population[i].successful_mutations[m] = pool[i].successful_mutations[m];
                new_population[i].failed_mutations[m] = pool[i].failed_mutations[m];
            }
        }

        // 2. Teraz bezpiecznie zwalniamy CAŁĄ pamięć dzieci
        for (int i = 0; i < total_children; i++) {
            free(children[i].genotype->genes);
            free(children[i].genotype);
            freeState(&children[i].state);
        }

        // 3. Zwalniamy starą populację
        for (int i = 0; i < starting_states; i++) {
            free(population[i].genotype->genes);
            free(population[i].genotype);
            freeState(&population[i].state);
        }
        free(population);

        // 4. Nowa populacja staje się główną populacją
        population = new_population;

        // Czyszczenie tablic pomocniczych
        free(children);
        free(pool);

        printf("Iteration %d | Best score: %d\n", iter, population[0].state.score);

        // POPRAWKA: Wywołanie customToCsv przekazujące wskaźnik na cały obiekt Entity populacji
        if(iter == 0 || iter == max_iterations-1){
            for (int i = 0; i < starting_states; i++) {
                char csv_name[64];
                sprintf(csv_name, "experiment_iter%d_rank%d.csv", iter, i);
                customToCsv(&input, population[i].genotype, csv_name);
            }
        }

        // Check patience criteria
        if (population[0].state.score > best_score) {
            best_score = population[0].state.score;
            patience_counter = 0; 
        } else {
            patience_counter++;
        }

        if (patience_counter >= patience) {
            printf("Early stopping: no improvement for %d iterations.\n", patience);
            for (int i = 0; i < starting_states; i++) {
                char csv_name[64];
                sprintf(csv_name, "experiment_iter%d_rank%d.csv", iter, i);
                customToCsv(&input, population[i].genotype, csv_name);
            }
            break;
        }
    }


    // --- MUTATION STATISTICS SUMMARY ---
    int selected_success[6] = {0, 0, 0, 0, 0, 0};
    int selected_fail[6] = {0, 0, 0, 0, 0, 0};

    for (int m = 0; m < 6; m++) {
        selected_success[m] += population[0].successful_mutations[m];
        selected_fail[m] += population[0].failed_mutations[m];
    }

    printf("\n======================== MUTATION STATISTICS ========================\n");
    printf("%-25s | %-16s | %-16s\n", "Mutation Name", "ALL ATTEMPTS", "SURVIVED SELECTION");
    printf("%-25s | %-7s / %-6s | %-7s / %-6s\n", "", "Success", "Failed", "Success", "Failed");
    printf("--------------------------------------------------------------------\n");
    //int all_mut=0;
    for (int m = 0; m < 6; m++) {
        //all_mut = all_mut + global_success[m] + global_fail[m];
        printf("%-25s | %-7d / %-6d | %-7d / %-6d\n", 
               MUTATION_NAMES[m], 
               global_success[m], global_fail[m], 
               selected_success[m], selected_fail[m]);
    }
    printf("====================================================================\n\n");
    //printf("===%d===", all_mut);
    // Final clean up
    for (int i = 0; i < starting_states; i++) {
        free(population[i].genotype->genes);
        free(population[i].genotype);
        freeState(&population[i].state);
    }
    free(population);
}

int main(void) {
    srand((unsigned int)time(NULL));

    Params test_params;
    test_params.alterOneGeneMutation  = 10;
    test_params.removeOneGeneMutation = 100;
    test_params.addOneGeneMutation    = 100;
    test_params.shiftOneGeneMutation  = 50;
    test_params.rotateOneGeneMutation = 50;
    test_params.clearAreaMutation     = 100;

    printf("Starting experiment with isolated entity tracking...\n");
    experiment(10, 100, 1000, 20, 10, test_params);

    return 0;
}