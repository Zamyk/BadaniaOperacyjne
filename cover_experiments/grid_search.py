import subprocess
import os
import sys
import itertools
import statistics
import uuid
from concurrent.futures import ThreadPoolExecutor, as_completed
import csv
import os

FILENAME = "data_logs.csv"

def log_data(value1, value2, value3, value4):
    """Appends 4 values to the CSV file. Creates the file if it doesn't exist."""
    file_exists = os.path.isfile(FILENAME)
    
    with open(FILENAME, mode="a", newline="", encoding="utf-8") as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow(["score", "width", "mut_remove", "mut_clear"])
            
        writer.writerow([value1, value2, value3, value4])
    print("Data logged successfully.")

def read_logs():
    """Reads and prints all data from the CSV file."""
    if not os.path.isfile(FILENAME):
        print("No log file found.")
        return

    with open(FILENAME, mode="r", encoding="utf-8") as file:
        reader = csv.reader(file)
        print("\n--- Reading Log File ---")
        for row in reader:
            print(row)
        print("------------------------\n")


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

def evaluate_params(params, num_runs=100):
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

        denominator = current_params["width"] * current_params["height"]
        
        avg_score = evaluate_params(current_params, num_runs=num_runs) / denominator

        log_data(avg_score, current_params["width"], current_params["mut_remove"], current_params["mut_clear"])

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
        "max_iterations": 1000,
        "patience": 50
    }
    
    # Najpierw ewaluacja parametrów domyślnych
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

    # Parametry do grid searcha
    param_grid = {
        "width": [2, 3, 4, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50],
        "height": [20],
        "preset": ["tetris"],
        "penalty": ["uniform"],
        "starting_states": [10],
        "duplications": [10],
        "mutations": [10],
        "mut_alter": [10],
        "mut_remove": [5, 10, 15, 20],
        "mut_add": [10],
        "mut_shift": [10],
        "mut_rotate": [10],
        "mut_clear": [5, 10, 15, 20]
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
