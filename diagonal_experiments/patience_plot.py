"""
Wykres: Średni score w zależności od parametru patience.

Porównuje dwie konfiguracje:
  - Domyślne parametry (score ~136)
  - Najlepsze parametry z grid searcha (score ~254)

Uruchomienie:
    python3 diagonal_experiments/patience_plot.py

Wynik zapisywany do: plots/score_vs_patience.png
"""

import subprocess
import os
import sys
import uuid
import statistics
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
from concurrent.futures import ThreadPoolExecutor, as_completed

# Dodajemy katalog nadrzędny, aby zaimportować parser.py
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from parser import ExperimentResult

BINARY_PATH = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', 'build', 'BadaniaOperacyjne')
)

# ── Konfiguracje ──────────────────────────────────────────────────────────────

# Stałe parametry bazowe (wspólne dla obu konfiguracji)
BASE_PARAMS = {
    "width":          15,
    "height":         15,
    "preset":         "tetris",
    "penalty":        "bad_diagonal",
    "max_iterations": 1000,
}

# Parametry domyślne (punkt odniesienia, ~136)
DEFAULT_PARAMS = {
    "starting_states": 10,
    "duplications":    100,
    "mutations":       40,
    "mut_alter":       10,
    "mut_remove":      100,
    "mut_add":         100,
    "mut_shift":       50,
    "mut_rotate":      50,
    "mut_clear":       100,
}

# Najlepsza konfiguracja znaleziona przez grid search (~254)
BEST_PARAMS = {
    "starting_states": 20,
    "duplications":    10,
    "mutations":       2,
    "mut_alter":       50,
    "mut_remove":      50,
    "mut_add":         150,
    "mut_shift":       50,
    "mut_rotate":      300,
    "mut_clear":       50,
}

# Wartości patience do przetestowania
PATIENCE_VALUES = [5, 10, 20, 30, 40, 50, 60, 80, 100, 120, 150, 200]

# Liczba powtórzeń na każdą wartość patience (więcej = dokładniejsza średnia)
NUM_RUNS = 10

# ── Funkcje pomocnicze ────────────────────────────────────────────────────────

def run_single(params: dict) -> float | None:
    """Uruchamia jeden eksperyment i zwraca best_score."""
    json_path = f"/tmp/patience_{uuid.uuid4().hex}.json"
    cmd = [BINARY_PATH, "--silent", "--output_json", json_path]
    for k, v in params.items():
        cmd.extend([f"--{k}", str(v)])

    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    if os.path.exists(json_path):
        res = ExperimentResult(json_path)
        os.remove(json_path)
        return res.best_score
    return None


def evaluate(params: dict, num_runs: int = NUM_RUNS):
    """
    Uruchamia num_runs eksperymentów równolegle.
    Zwraca (średnia, odchylenie_standardowe).
    """
    scores = []
    with ThreadPoolExecutor(max_workers=num_runs) as executor:
        futures = [executor.submit(run_single, params) for _ in range(num_runs)]
        for f in as_completed(futures):
            val = f.result()
            if val is not None:
                scores.append(val)

    if not scores:
        return float('nan'), 0.0
    mean = statistics.mean(scores)
    std  = statistics.stdev(scores) if len(scores) > 1 else 0.0
    return mean, std


# ── Zbieranie danych ──────────────────────────────────────────────────────────

def collect_data():
    best_means,    best_stds    = [], []
    default_means, default_stds = [], []

    for patience in PATIENCE_VALUES:
        # Najlepsze parametry + bieżąca patience
        p_best = {**BASE_PARAMS, **BEST_PARAMS, "patience": patience}
        m, s = evaluate(p_best)
        best_means.append(m);  best_stds.append(s)
        print(f"[BEST]    patience={patience:4d}  →  mean={m:.1f}  ±  {s:.1f}")

        # Domyślne parametry + bieżąca patience
        p_def = {**BASE_PARAMS, **DEFAULT_PARAMS, "patience": patience}
        m, s = evaluate(p_def)
        default_means.append(m);  default_stds.append(s)
        print(f"[DEFAULT] patience={patience:4d}  →  mean={m:.1f}  ±  {s:.1f}")

    return (
        np.array(best_means),    np.array(best_stds),
        np.array(default_means), np.array(default_stds),
    )


# ── Rysowanie wykresu ─────────────────────────────────────────────────────────

def plot(patience_values, best_means, best_stds, default_means, default_stds,
         output_path="plots/score_vs_patience.png"):

    plt.style.use('dark_background')
    fig, ax = plt.subplots(figsize=(12, 6.5))
    fig.patch.set_facecolor('#0f0f1a')
    ax.set_facecolor('#0f0f1a')

    x = np.array(patience_values)

    # ── Domyślne parametry ──
    ax.plot(x, default_means,
            color='#FF6B6B', linewidth=2, marker='o', markersize=6,
            label='Domyślne parametry', zorder=3)
    ax.fill_between(x,
                    default_means - default_stds,
                    default_means + default_stds,
                    alpha=0.18, color='#FF6B6B')

    # ── Najlepsze parametry (GS) ──
    ax.plot(x, best_means,
            color='#4ECDC4', linewidth=2.5, marker='D', markersize=7,
            label='Najlepsze parametry (Grid Search)', zorder=4)
    ax.fill_between(x,
                    best_means - best_stds,
                    best_means + best_stds,
                    alpha=0.18, color='#4ECDC4')

    # ── Poziome linie referencyjne (average z eksperymentów GS) ──
    refs = [
        ("Domyślne średnie (136)",        136, '#FF6B6B', '--'),
        ("Po 1. Grid Search (207)",        207, '#FFD93D', ':'),
        ("Najlepszy Grid Search (254)",     254, '#4ECDC4', '--'),
    ]
    for label, score, color, ls in refs:
        ax.axhline(score, color=color, linestyle=ls,
                   linewidth=1.0, alpha=0.45, label=f'Ref: {label}')

    # ── Kosmetyka ──
    ax.set_xlabel("patience", fontsize=13, labelpad=8, color='#cccccc')
    ax.set_ylabel("Średni best_score  (±1 std, cień)", fontsize=13, labelpad=8, color='#cccccc')
    ax.set_title(
        f"Wpływ parametru patience na jakość rozwiązania\n"
        f"({NUM_RUNS} powtórzeń na punkt · 15×15 · tetris · bad_diagonal)",
        fontsize=15, pad=14, color='white', fontweight='bold'
    )

    ax.xaxis.set_major_locator(ticker.FixedLocator(patience_values))
    ax.tick_params(axis='x', rotation=45, colors='#aaaaaa')
    ax.tick_params(axis='y', colors='#aaaaaa')
    ax.yaxis.set_major_locator(ticker.MultipleLocator(25))

    ax.grid(axis='y', linestyle='--', alpha=0.25, color='#888888')
    ax.grid(axis='x', linestyle=':', alpha=0.15,  color='#888888')
    ax.set_axisbelow(True)

    for spine in ax.spines.values():
        spine.set_edgecolor('#444444')

    legend = ax.legend(framealpha=0.3, fontsize=9.5, loc='lower right',
                       facecolor='#1a1a2e', edgecolor='#555577')
    ax.set_xlim(patience_values[0] - 3, patience_values[-1] + 3)

    fig.tight_layout()
    os.makedirs(os.path.dirname(output_path) or ".", exist_ok=True)
    plt.savefig(output_path, dpi=150, bbox_inches='tight', facecolor='#0f0f1a')
    print(f"\nWykres zapisany: {output_path}")


# ── Główna funkcja ────────────────────────────────────────────────────────────

def main():
    if not os.path.exists(BINARY_PATH):
        print(f"Błąd: Nie znaleziono binarki:\n  {BINARY_PATH}")
        print("Skompiluj projekt (make lub cmake --build build) i spróbuj ponownie.")
        sys.exit(1)

    print("=" * 62)
    print("  Wykres: score vs patience")
    print(f"  Patience values : {PATIENCE_VALUES}")
    print(f"  Powtórzeń/punkt : {NUM_RUNS}")
    print("=" * 62)

    best_means, best_stds, default_means, default_stds = collect_data()

    output_path = os.path.join(
        os.path.dirname(__file__), '..', 'plots', 'score_vs_patience.png'
    )
    plot(PATIENCE_VALUES, best_means, best_stds, default_means, default_stds,
         output_path=output_path)


if __name__ == "__main__":
    main()
