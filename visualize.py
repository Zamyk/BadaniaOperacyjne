import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from datetime import datetime

def visualize_board(csv_path, name=""):
    # Load the board, assuming no header
    try:
        board = pd.read_csv(csv_path, header=None).values
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return

    plt.figure(figsize=(8, 8))
    
    # Use 'tab20' for distinct colors or 'Set3'
    # We use 'None' for interpolation to keep the blocks sharp
    cmap = plt.get_cmap('tab20', np.max(board) + 2) 
    
    # Set the color for -1 (empty) to light grey
    cmap.set_under('lightgrey')

    # Display the board
    # vmin=0 ensures that -1 is treated as "below" the range (showing lightgrey)
    im = plt.imshow(board, cmap=cmap, vmin=0, interpolation='nearest')

    # Add a grid for clarity
    plt.xticks(np.arange(-0.5, board.shape[1], 1), [])
    plt.yticks(np.arange(-0.5, board.shape[0], 1), [])
    plt.grid(color='white', linestyle='-', linewidth=2)

    plt.title(f"Polyomino Board Visualization ({board.shape[1]}x{board.shape[0]})")
    plt.colorbar(im, ticks=range(int(np.max(board)) + 1), label="Polyomino ID")
    plt.savefig(f"{name}plot_{datetime.now().strftime("%Y-%m-%d_%H-%M-%S")}.png")

if __name__ == "__main__":
    visualize_board('out_prev.csv', "prev_")
    visualize_board('out.csv')