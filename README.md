# Badania Operacyjne – Pokrywanie planszy polyominami

Projekt na Badania Operacyjne. Cel: optymalne pokrycie prostokątnej planszy klockami (polyominami) z uwzględnieniem wartości klocków i kar za niepokryte pola.

## Co jest zrobione

- **Silnik** (`engine.h` / `engine.c`) — struktury danych (`Polyomino`, `Input`, `State`), obsługa rotacji klocków (4 kierunki), dodawanie/usuwanie z planszy, obliczanie wyniku (wartości − kary), eksport do CSV
- **Zachłanne wypełnianie** (`main.c`) — zachłanne wypełnianie planszy (brute-force po pozycjach i rotacjach)
- **Przykładowy input** — plansza 10×10, trzy typy klocków (T, L, S), po 8 sztuk każdego
- **Wizualizacja** (`visualize.py`) — rysowanie planszy z `out.csv` za pomocą matplotlib

## Budowanie i uruchomienie

Jest cmake, można albo jakoś z terminala, albo vs-code ma wtyczkę.

## TODO
- Generowanie większego, losowego inputu
- Implementacja algorytmu ewolucyjnego - myślę, żeby zacząć bez krzyżowania - tylko mutacje
