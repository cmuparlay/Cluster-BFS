import pandas as pd 
import numpy as np
import os
from graph import graph_map

def format_digit(num):
    if pd.isna(num):
        return ''
    if num > 100:
        return f"{int(num)}"
    elif 10 <= num < 100:
        return f"{num:.1f}"
    else:
        return f"{num:.2f}"

def draw_table(df, small_file, full_file):
  new_df = pd.DataFrame()
  new_df['Dataset']=df['Data'].apply(lambda a: graph_map[a])
  new_df["BaseT"]=df['BaseT'].apply(format_digit)
  for w in [64,32,16,8]:
    new_df[f"CC{w}T"]=df[f'CC{w}T'].apply(format_digit)
  new_df["BaseE"]=(df["BaseE"]*100).apply(lambda a: f"{a:.1f}")
  for w in [64,32,16,8]:
    new_df[f"CC{w}E"]=(df[f"CC{w}E"]*100).apply(lambda a: f"{a:.1f}")
  new_df["BaseQ"]=(df["BaseQ"]*1000).apply(lambda a: f"{a:.1f}")
  for w in [64,32,16,8]:
    new_df[f"CC{w}Q"]=(df[f"CC{w}Q"]*1000).apply(lambda a: f"{a:.1f}")
  
  small_columns=["Dataset","BaseT","CC64T","CC8T","BaseE","CC64E","CC8E","BaseQ","CC64Q","CC8Q"]
  small_df=new_df[small_columns]

  multi_index = pd.MultiIndex.from_tuples([
    ('', 'Data'), 
    ('Index Time(s)', 'Plain'),  
    ('Index Time(s)', f'$w$=64'), 
    ('Index Time(s)', f'$w$=32'),
    ('Index Time(s)', f'$w$=16'),
    ('Index Time(s)', f'$w$=8'),   
    ('$\epsilon$(\%)', 'Plain'),
    ('$\epsilon$(\%)', f'$w$=64'),
    ('$\epsilon$(\%)', f'$w$=32'),
    ('$\epsilon$(\%)', f'$w$=16'),
    ('$\epsilon$(\%)', f'$w$=8'),
    ('Query Time(ms)', 'Plain'),
    ('Query Time(ms)', f'$w$=64'),
    ('Query Time(ms)', f'$w$=32'),
    ('Query Time(ms)', f'$w$=16'),
    ('Query Time(ms)', f'$w$=8'),
  ])
  new_df.columns=multi_index
  with open(full_file,'w') as tf:
    tf.write(new_df.to_latex(index=False, column_format='l|@{ }c@{ }c@{ }c@{ }c@{ }c@{ }|@{ }r@{ }r@{ }r@{ }r@{ }r@{ }|@{ }c@{ }c@{ }c@{ }c@{ }c@{ }', escape=False))

  small_multi_index = pd.MultiIndex.from_tuples([
    ('', 'Data'), 
    ('Index Time(s)', 'Plain'),  
    ('Index Time(s)', f'$w=64$'), 
    ('Index Time(s)', f'$w=8$'), 
    ('$\epsilon$(\%)', 'Plain'),
    ('$\epsilon$(\%)', f'$w=64$'),
    ('$\epsilon$(\%)', f'$w=8$'),
    ('Query Time(ms)', 'Plain'),
    ('Query Time(ms)', f'$w=64$'),
    ('Query Time(ms)', f'$w=8$'),
  ])
  small_df.columns=small_multi_index
  with open(small_file,'w') as tf:
    tf.write(small_df.to_latex(index=False, column_format='l|ccc|rrr|ccc', escape=False))
  
  

if __name__ == '__main__':
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp4.csv')
  small_file=f"{CURRENT_DIR}/../figs_and_tables/exp4_small_table.tex"
  full_file=f"{CURRENT_DIR}/../figs_and_tables/exp4_full_table.tex"
  draw_table(df, small_file, full_file)