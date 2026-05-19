import pandas as pd
import matplotlib.pyplot as plt

def plot_all_points_with_max_labels(file_path):
    df = pd.read_csv(file_path)
    df['x'] = df['width'] * 10
    
    plt.figure(figsize=(12, 7))
    
    plt.scatter(df['x'], df['score'], color='blue', alpha=0.5, label='punkty')
    
    max_scores = df.loc[df.groupby('x')['score'].idxmax()]
    
    for _, row in max_scores.iterrows():
        label = f"{row['mut_remove']}/{row['mut_clear']}"
        plt.annotate(label, 
                     (row['x'], row['score']), 
                     textcoords="offset points", 
                     xytext=(0, 10), 
                     ha='center', 
                     fontsize=9, 
                     color='darkred',
                     fontweight='bold')
    
    plt.scatter(max_scores['x'], max_scores['score'], color='red', s=50, label='punkty o najwyższym score')
    
    plt.ylim(-0.22, 0.02)
    
    plt.xticks(sorted(df['x'].unique()))
    
    plt.xlabel('liczba kratek (width * height)')
    plt.ylabel('znormalizowany score')
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend()
    
    plt.savefig("data_logs.png", dpi=300)
    plt.close()

plot_all_points_with_max_labels("data_logs.csv")