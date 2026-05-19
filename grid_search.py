import subprocess
import os
import itertools
from parser import ExperimentResult

def run_experiment(width, height, preset, penalty, starting_states, duplications, max_iter, patience,
                   mut_alter, mut_remove, mut_add, mut_shift, mut_rotate, mut_clear, output_json):
                   
    cmd = [
        "./build/BadaniaOperacyjne",
        "--width", str(width),
        "--height", str(height),
        "--preset", preset,
        "--penalty", penalty,
        "--starting_states", str(starting_states),
        "--duplications", str(duplications),
        "--max_iterations", str(max_iter),
        "--patience", str(patience),
        "--mut_alter", str(mut_alter),
        "--mut_remove", str(mut_remove),
        "--mut_add", str(mut_add),
        "--mut_shift", str(mut_shift),
        "--mut_rotate", str(mut_rotate),
        "--mut_clear", str(mut_clear),
        "--output_json", output_json
    ]
    
    # Run the compiled binary
    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    # Parse the results
    if os.path.exists(output_json):
        res = ExperimentResult(output_json)
        # Opcjonalnie usunięcie pliku po zaczytaniu, żeby nie zaśmiecać dysku
        os.remove(output_json)
        return res
    return None

def main():
    print("Rozpoczynam Grid Search...")
    
    # Parametry stałe dla środowiska zgodnie z życzeniem
    WIDTH = 15
    HEIGHT = 15
    PRESET = "tetris"
    PENALTY = "bad_diagonal"
    
    # Parametry stałe dla ogólnego solvera, żeby wykonało się w miarę szybko
    STARTING_STATES = 10
    DUPLICATIONS = 50
    MAX_ITER = 300
    PATIENCE = 15
    
    # Parametry przeszukiwane (wagi mutacji)
    # Zmniejszamy siatkę żeby działało to sensownym czasie
    mut_add_options = [50, 150]
    mut_remove_options = [10, 100]
    mut_shift_options = [50, 100]
    
    best_score = float('-inf')
    best_config = None
    
    total_combinations = len(mut_add_options) * len(mut_remove_options) * len(mut_shift_options)
    current = 0
    
    for add_w, remove_w, shift_w in itertools.product(mut_add_options, mut_remove_options, mut_shift_options):
        current += 1
        print(f"[{current}/{total_combinations}] Testowanie wag: add={add_w}, remove={remove_w}, shift={shift_w}...")
        
        # Pozostałe mutacje na sztywno, żeby ograniczyć złożoność przeszukiwania
        res = run_experiment(
            width=WIDTH, height=HEIGHT, preset=PRESET, penalty=PENALTY,
            starting_states=STARTING_STATES, duplications=DUPLICATIONS,
            max_iter=MAX_ITER, patience=PATIENCE,
            mut_alter=10, mut_remove=remove_w, mut_add=add_w,
            mut_shift=shift_w, mut_rotate=50, mut_clear=50,
            output_json="temp_grid_result.json"
        )
        
        if res:
            print(f"   Uzyskany wynik: {res.best_score} (w {res.num_iterations} iteracjach)")
            if res.best_score > best_score:
                best_score = res.best_score
                best_config = res.config
                print("   *** Nowy najlepszy wynik! ***")
        else:
            print("   Błąd wykonania (brak pliku JSON).")
            
    print("\n" + "="*50)
    print("GRID SEARCH ZAKOŃCZONY!")
    print(f"Najlepszy znaleziony score: {best_score}")
    print("Najlepsza konfiguracja wag mutacji:")
    if best_config:
        print(f" - Add: {best_config['mut_add']}")
        print(f" - Remove: {best_config['mut_remove']}")
        print(f" - Shift: {best_config['mut_shift']}")
        
if __name__ == "__main__":
    main()
