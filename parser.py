import json

class ExperimentResult:
    def __init__(self, json_path):
        with open(json_path, 'r') as f:
            data = json.load(f)
            
        self.config = data.get("config", {})
        self.scores = data.get("scores", [])
        self.mutation_stats = data.get("mutation_stats", {})
        
    @property
    def best_score(self):
        """Zwraca najlepszy (ostatni lub maksymalny) score z iteracji."""
        if not self.scores:
            return float('-inf')
        return max(self.scores)
        
    @property
    def num_iterations(self):
        """Zwraca liczbę rzeczywistych iteracji przed zatrzymaniem."""
        return len(self.scores)
        
    def get_mutation_success_rate(self, mutation_name):
        """Zwraca procentowy sukces danej mutacji."""
        stats = self.mutation_stats.get(mutation_name)
        if not stats:
            return 0.0
            
        total = stats["success"] + stats["fail"]
        if total == 0:
            return 0.0
            
        return stats["success"] / total
        
    def __str__(self):
        return f"ExperimentResult(Best Score: {self.best_score}, Iterations: {self.num_iterations})"

if __name__ == "__main__":
    # Testowy kod, jeśli skrypt odpalony bezpośrednio
    print("Parser gotowy do użycia.")
