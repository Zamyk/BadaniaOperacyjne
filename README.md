# Badania Operacyjne – Optymalizacja Pokrywania Planszy Poliominami

Projekt w ramach przedmiotu Badania Operacyjne. Głównym celem jest optymalne pokrycie dwuwymiarowej planszy za pomocą zestawu poliomin, z uwzględnieniem wartości punktowych klocków oraz zróżnicowanych kar ujemnych nakładanych za niepokryte pola lub wykorzystywanie zabronionych stref (np. przeszkód).

Projekt pierwotnie oparty o heurystyki zachłanne, został w pełni przekształcony w zaawansowany **algorytm ewolucyjny (genetyczny)** pracujący z losowymi mutacjami i adaptacyjnym modelem uczącym się wag tych mutacji.

## Główne Funkcjonalności

- **Silnik Reguł (`engine.h`, `engine.c`)**: Obsługa struktury planszy, weryfikacja kolizji, zliczanie wyników, rotacje klocków. Posiada wbudowane zróżnicowane generatory środowiska:
  - Presety plansz: `tetris` (klasyczne L, T, S), `simple`, `random`, `obstacle` (zbudowana wokół zabójczej przeszkody), `irregular` (wymagająca asymetryczna geometria).
  - Typy kar (Penalties): `uniform` (równe na całej mapie), `bad_corners`, `obstacle` (wysokie kary w centrum), `modulo` (zróżnicowany krajobraz ujemnych kar punktowych).
- **Algorytm Ewolucyjny (`main.c`)**: Zastąpił proste zachłanne układanie. Operuje na populacji, wprowadzając szeroki wachlarz mutacji dla każdego kroku (dodawanie, usuwanie, podmienianie, przesuwanie, rotacja, obszarowe czyszczenie). 
- **Adaptacyjne Wagi Mutacji**: System posiada możliwość włączenia dynamicznych wag operacji ewolucyjnych -- jeśli mutacja (np. przesunięcie w dół) da dobry rezultat dla klocka, system podwyższa prawdopodobieństwo jej losowania w kolejnym pokoleniu, stopniowo je jednak "wygaszając" z upływem czasu dla zachowania balansu.
- **Rozbudowana Wizualizacja (`visualize.py`)**: Rozbudowany skrypt wykorzystujący `matplotlib` i `pandas` do kreślenia pięknych, finalnych reprezentacji stanu planszy oraz wykresów ewolucji wag. Oparty na systemie eksportowania logów do spersonalizowanych formatów `.csv`.

## Budowanie

Zalecanym sposobem budowania w środowiskach Unixowych/WSL jest użycie dołączonego pliku `Makefile`. Wystarczy wywołać w terminalu:

```bash
make
```

Alternatywnie, projekt można zbudować wywołując kompilator ręcznie:

```bash
gcc main.c engine.c -o main
```

*(W repozytorium znajduje się również konfiguracja CMake, pozwalająca na budowanie przy użyciu nowoczesnych IDE).*

## Uruchamianie (Opcje CLI)

Program może zostać włączony bez podawania argumentów – poprosi wówczas o kluczowe parametry poprzez konsolę. Uruchomienie z flagami wiersza poleceń jest zalecane i daje kontrolę nad każdym aspektem środowiska.

Przykładowe uruchomienie eksperymentu dla planszy przeszkód z włączoną adaptacją wag:
```bash
./main --width 12 --height 12 --preset obstacle --penalty obstacle --prefix obstacle_run --adapt_weights
```

**Pełna lista najpopularniejszych argumentów:**
- `--width <n>`, `--height <n>`: Wymiary planszy.
- `--preset <nazwa>`: Konfiguracja bloków (`tetris`, `simple`, `random`, `obstacle`, `irregular`).
- `--penalty <nazwa>`: Funkcja wyliczania kar (`uniform`, `bad_diagonal`, `checkerboard`, `obstacle`, `modulo` i inne).
- `--prefix <nazwa>`: Przedrostek dla plików wyjściowych (zapisze finalną planszę jako `prefix.csv` oraz wagi jako `weights_prefix.csv`).
- `--adapt_weights`: Flaga włączająca inteligentny system uczenia się ewolucji z nagradzaniem i karaniem operatorów mutacji (domyślnie **wyłączone** -- statyczne algorytmy wg `test_params`).
- Konfiguracja ewolucji: `--starting_states`, `--duplications`, `--max_iterations`, `--mutations`, `--patience`.

## Wizualizacja Wyników

Uruchomienie skryptów wizualizacyjnych wykonuje się przy użyciu języka Python. Zakładając, że główny program zapisał wynikowe CSV pod nazwą z parametru `--prefix`:

```bash
# Wyrysowanie planszy
python3 -c "import visualize; visualize.visualize_board('nazwa_pliku.csv')"

# Wyrysowanie wykresów spadku/wzrostu wag adaptacyjnych:
python3 visualize.py weights
```

Wszystkie wygenerowane rysunki `.png` zostaną umieszczone w utworzonym automatycznie folderze `plots/`.
