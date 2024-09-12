import pandas as pd 
import numpy as np
from graph import graph_map
import os

def format_digit3(num):
    if pd.isna(num):
        return ''
    if num > 100:
        return f"{int(num)}"
    elif 10 <= num < 100:
        return f"{num:.1f}"
    else:
        return f"{num:.2f}"
def format_digit4(num):
    if pd.isna(num):
        return ''
    if num > 1000:
        return f"{int(num)}"
    elif 100 <= num < 1000:
        return f"{num:.1f}"
    elif 10<=num<100:
        return f"{num:.2f}"
    else:
        return f"{num:.3f}"
def format_KM(num):
    if num >= 1e9:
        return f"{format_digit3(num / 1e9)}B"
    if num >= 1e6:
        return f"{format_digit3(num / 1e6)}M"
    elif num >= 1e3:
        return f"{format_digit3(num / 1e3)}K"
    else:
        return str(format_digit3(num))
def geo_mean(iterable):
    a = np.array(iterable)
    return a.prod()**(1.0/len(a))

def draw_table1(df, outfile):
    pd.set_option('display.max_colwidth', None)
    new_df = pd.DataFrame()
    #   print(df['std_serial']/10)
    times = "$\times$"
    new_df["Data"]=df['Dataset'].apply(lambda a: graph_map[a])
    new_df['n']=df['n'].apply(format_KM)
    new_df['m']=df['m'].apply(format_KM)
    new_df["64 seq BFS"]=(df['sequential_BFS_seq']).apply(format_digit3)
    new_df["Akiba"]=(df['sequential_BFS_seq']/df['Akiba_BFS_seq']).apply(lambda a: f"{format_digit3(a)}{times}")
    new_df["Ligra"]=(df["sequential_BFS_seq"]/df['ligra_BFS']).apply(lambda a: f"{format_digit3(a)}{times}")
    new_df["Final"]=(df['sequential_BFS_seq']/df['cluster_BFS']).apply(lambda a: f"{format_digit3(a)}{times}")
    new_df["CCBFS time"]=(df['cluster_BFS']).apply(format_digit4)
    new_df["ligra self spd."]=(df['ligra_BFS_seq']/df['ligra_BFS']).apply(lambda a: f"{format_digit3(a)}{times}")
    new_df["ccbfs self spd."]=(df['cluster_BFS_seq']/df['cluster_BFS']).apply(lambda a: f"{format_digit3(a)}{times}")
    
    
    #   new_df["CCBFS"]=(df['par ligra BFS']/df['par CCBFS']).apply(format_digit)
    GeoMean=dict()
    GeoMean["Data"]="GeoMean"
    GeoMean["n"]=""
    GeoMean["m"]=""
    GeoMean["64 seq BFS"]=format_digit3(geo_mean((df['sequential_BFS_seq'])))
    GeoMean["Akiba"]=f"{format_digit3(geo_mean((df['sequential_BFS_seq']/df['Akiba_BFS_seq'])))}{times}"
    GeoMean["Ligra"]=f"{format_digit3(geo_mean((df['sequential_BFS_seq']/df['ligra_BFS'])))}{times}"
    GeoMean["Final"]=f"{format_digit3(geo_mean((df['sequential_BFS_seq']/df['cluster_BFS'])))}{times}"
    GeoMean["CCBFS time"]=f"{format_digit4(geo_mean(df['cluster_BFS']))}"
    GeoMean["ligra self spd."]=f"{format_digit3(geo_mean(df['ligra_BFS_seq']/df['ligra_BFS']))}{times}"
    GeoMean["ccbfs self spd."]=f"{format_digit3(geo_mean(df['cluster_BFS_seq']/df['cluster_BFS']))}{times}"
    
    
    #   print(GeoMean)
    new_df=new_df.append(GeoMean,ignore_index=True)
    #   print(new_df)

    multi_index = pd.MultiIndex.from_tuples([
        ('', 'Dataset'), 
        ('Graph Information', '$n$'), 
        ('Graph Information', '$m$'), 
        ('Seq-BFS', 'Time(s)'), 
        ('Related Work', 'AIY'), 
        ('Related Work', 'Ligra'), 
        ('Parallel C-BFS', 'Final'),
        ('Parallel C-BFS', 'Time(s)'),
        ('Self-Speedup', 'C-BFS'),
        ('Self-Speedup', 'Ligra'), 
    ])
    #   new_df.columns = [f'\\textbf{{{col}}}' for col in new_df.columns]
    new_df.columns=multi_index
    with open(outfile,'w') as tf:
        tf.write(new_df.to_latex(index=False, column_format='l|cc|c|cc|cc|cc', escape=False))
if __name__ == '__main__':
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  outfile=f"{CURRENT_DIR}/../figs_and_tables/exp1_table.tex"
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp1.csv')
  draw_table1(df,outfile)
  