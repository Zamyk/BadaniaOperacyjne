import os
import glob
import re
import matplotlib.pyplot as plt
plt.style.use('dark_background')
import numpy as np

def rotate_point(x, y, rotation):
    """
    Dokładne odwzorowanie getPolyominoPoint() z C.

    Enum:
    UP    = 0
    LEFT  = 1
    RIGHT = 2
    DOWN  = 3
    """

    if rotation == 0:      # UP
        return x, y

    elif rotation == 1:    # LEFT
        return y, -x

    elif rotation == 2:    # RIGHT
        return -y, x

    elif rotation == 3:    # DOWN
        return -x, -y

    return x, y

def parse_custom_csv(csv_path):
    metadata = {}
    poly_types = {}
    penalties = []
    placed_polyominoes = []
    current_section = None
    
    with open(csv_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith('['):
                current_section = line
                continue
                
            tokens = line.split(',')
            
            if current_section == '[METADATA]':
                metadata[tokens[0]] = int(tokens[1])
            elif current_section == '[POLYOMINO_TYPES]':
                type_id = int(tokens[0])
                n_points = int(tokens[1])
                coords = []
                for i in range(n_points):
                    coords.append((int(tokens[2 + i*2]), int(tokens[3 + i*2])))
                poly_types[type_id] = coords
            elif current_section == '[PENALTIES]':
                penalties.append([int(x) for x in tokens])
            elif current_section == '[PLACED_POLYOMINOES]':
                placed_polyominoes.append({
                    'unique_id': int(tokens[0]),
                    'type_id': int(tokens[1]),
                    'x': int(tokens[2]),
                    'y': int(tokens[3]),
                    'rotation': int(tokens[4])
                })
                
    return metadata, poly_types, np.array(penalties), placed_polyominoes

def visualize_board(csv_path, output_dir="plots"):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    file_name = os.path.basename(csv_path)
    match = re.search(r"experiment_iter(\d+)_rank(\d+)\.csv", file_name)
    if not match:
        return 

    iteration, rank = match.group(1), match.group(2)

    try:
        metadata, poly_types, penalty_board, placed_list = parse_custom_csv(csv_path)
    except Exception as e:
        print(f"Error parsing CSV {file_name}: {e}")
        return

    height = metadata['height']
    width = metadata['width']
    poly_board = np.full((height, width), -1, dtype=int)

    for poly in placed_list:
        u_id = poly['unique_id']
        t_id = poly['type_id']
        orig_x = poly['x']
        orig_y = poly['y']
        rot_enum = poly['rotation']

        if t_id not in poly_types:
            continue

        for pt_x, pt_y in poly_types[t_id]:
            rot_x, rot_y = rotate_point(pt_x, pt_y, rot_enum)
            final_x = orig_x + rot_x
            final_y = orig_y + rot_y

            if 0 <= final_x < width and 0 <= final_y < height:
                poly_board[final_y, final_x] = u_id

    fig, ax = plt.subplots(figsize=(9, 9))

    if len(placed_list) > 0:
        max_u_id = max([p['unique_id'] for p in placed_list])
        poly_cmap = plt.get_cmap('tab20', max_u_id + 2)
        masked_poly = np.ma.masked_where(poly_board < 0, poly_board)
        ax.imshow(masked_poly, cmap=poly_cmap, vmin=0, interpolation='nearest')

        for y in range(height):
            for x in range(width):
                current_id = poly_board[y, x]
                if current_id >= 0:
                    if x + 1 < width and poly_board[y, x + 1] != current_id:
                        ax.plot([x + 0.5, x + 0.5], [y - 0.5, y + 0.5], color='white', linewidth=3)
                    if y + 1 < height and poly_board[y + 1, x] != current_id:
                        ax.plot([x - 0.5, x + 0.5], [y + 0.5, y + 0.5], color='white', linewidth=3)
                    if x == 0 or poly_board[y, x - 1] != current_id:
                        ax.plot([x - 0.5, x - 0.5], [y - 0.5, y + 0.5], color='white', linewidth=3)
                    if y == 0 or poly_board[y - 1, x] != current_id:
                        ax.plot([x - 0.5, x + 0.5], [y - 0.5, y - 0.5], color='white', linewidth=3)

    for y in range(height):
        for x in range(width):
            val = penalty_board[y, x]
            text_color = "white" if poly_board[y, x] < 0 else "black"
            ax.text(x, y, int(val), ha="center", va="center", color=text_color, fontsize=9, weight='bold')

    ax.set_xticks(np.arange(-0.5, width, 1))
    ax.set_yticks(np.arange(-0.5, height, 1))
    ax.set_xticklabels([])
    ax.set_yticklabels([])
    ax.grid(color='grey', linestyle=':', linewidth=0.5)

    plt.title(f"Polyomino Board - Iteration {iteration} | Rank {rank}")
   
    output_path = os.path.join(output_dir, f"plot_iter{iteration}_rank{rank}.png")
    plt.savefig(output_path, bbox_inches='tight')
    plt.close()

def process_all_experiment_csvs(search_directory):
    search_pattern = os.path.join(search_directory, "experiment_iter*_rank*.csv")
    csv_files = glob.glob(search_pattern)
    
    if not csv_files:
        print(f"No matching CSV files found in: {search_directory}")
        return

    print(f"Found {len(csv_files)} files to process...")
    csv_files.sort(key=lambda f: [int(s) for s in re.findall(r'\d+', os.path.basename(f))])

    for file_path in csv_files:
        print(f"Processing: {os.path.basename(file_path)}")
        visualize_board(file_path, output_dir="plots")
    print("Visualization finished!")

if __name__ == "__main__":
    process_all_experiment_csvs(".")