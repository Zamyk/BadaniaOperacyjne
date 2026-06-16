import subprocess
import os
import sys
import itertools
import statistics
import uuid
from concurrent.futures import ThreadPoolExecutor, as_completed

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from parser import ExperimentResult

BINARY_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'BadaniaOperacyjne'))

def run_experiment_single(params, output_json):
    cmd = [BINARY_PATH, "--silent", "--output_json", output_json]
    for k, v in params.items():
        cmd.extend([f"--{k}", str(v)])
        
    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    if os.path.exists(output_json):
        res = ExperimentResult(output_json)
        os.remove(output_json)
        return res
    return None

def evaluate_params(params, num_runs=10):
    def single_run(_):
        json_path = f"/tmp/grid_result_{uuid.uuid4().hex}.json"
        res = run_experiment_single(params, json_path)
        return res.best_score if res is not None else None

    scores = []
    with ThreadPoolExecutor(max_workers=num_runs) as executor:
        futures = [executor.submit(single_run, i) for i in range(num_runs)]
        for f in as_completed(futures):
            val = f.result()
            if val is not None:
                scores.append(val)

    if not scores:
        return float('-inf')
    return statistics.mean(scores)

def grid_search(base_params, param_grid, num_runs=10):
    keys = list(param_grid.keys())
    values = list(param_grid.values())
    
    combinations = list(itertools.product(*values))
    total = len(combinations)
    
    best_score = float('-inf')
    best_params = None
    
    print(f"Rozpoczynam Grid Search ({total} kombinacji do przetestowania).")
    
    for idx, combo in enumerate(combinations, 1):
        current_params = base_params.copy()
        for k, v in zip(keys, combo):
            current_params[k] = v
            
        combo_str = ", ".join(f"{k}={v}" for k, v in zip(keys, combo))
        print(f"[{idx}/{total}] Testowanie: {combo_str}")
        
        avg_score = evaluate_params(current_params, num_runs=num_runs)
        print(f"   Średni wynik z {num_runs} uruchomień: {avg_score:.2f}")
        
        if avg_score > best_score:
            best_score = avg_score
            best_params = current_params.copy()
            print("   *** Nowy najlepszy średni wynik! ***")
            
    return best_score, best_params

def main():
    if not os.path.exists(BINARY_PATH):
        print(f"Błąd: Nie znaleziono skompilowanego programu w {BINARY_PATH}.")
        print("Skrypt należy uruchamiać np. komendą: python3 diagonal_experiments/grid_search.py z głównego katalogu.")
        sys.exit(1)

    base_params = {
        "width": 15,
        "height": 15,
        "preset": "tetris",
        "penalty": "bad_diagonal",
        "max_iterations": 1000,
        "patience": 50
    }
    
    default_params = base_params.copy()
    default_params.update({
        "starting_states": 10,
        "duplications": 100,
        "mutations": 40,
        "mut_alter": 10,
        "mut_remove": 100,
        "mut_add": 100,
        "mut_shift": 50,
        "mut_rotate": 50,
        "mut_clear": 100
    })
    
    print("========================================")
    print("Ewaluacja parametrów domyślnych (10 prób)...")
    default_score = evaluate_params(default_params, num_runs=10)
    print(f"Średni wynik domyślny: {default_score:.2f}")
    print("========================================\n")
    
    param_grid = {
        "starting_states": [10],
        "duplications": [10],
        "mutations": [20, 40],
        "mut_alter": [50, 100],
        "mut_remove": [50, 100],
        "mut_add": [50, 100],
        "mut_shift": [50, 100],
        "mut_rotate": [50, 100],
        "mut_clear": [50, 100]
    }
    
    best_score, best_config = grid_search(base_params, param_grid, num_runs=10)
    
    print("\n" + "="*50)
    print("GRID SEARCH ZAKOŃCZONY!")
    print(f"Najlepszy znaleziony średni score: {best_score:.2f}")
    print("Najlepsza konfiguracja:")
    if best_config:
        for k, v in best_config.items():
            if k in param_grid:
                print(f" - {k}: {v}")

if __name__ == "__main__":
    main()
