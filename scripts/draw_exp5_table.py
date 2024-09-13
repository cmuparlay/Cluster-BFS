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

def draw_ADO_d_table(df, outfile):
  new_df = pd.DataFrame()
  # df['Dataset'] = df['Dataset'].str.replace('_', '\\_', regex=False)
  new_df['Dataset']=df['Data'].apply(lambda a: graph_map[a])
  new_df["BaseT"]=df['BaseT'].apply(format_digit)
  new_df["2T"]=df['D2T'].apply(format_digit)
  new_df["3T"]=df["D3T"].apply(format_digit)
  new_df["4T"]=df["D4T"].apply(format_digit)
  new_df["BaseE"]=(df["BaseE"]*100).apply(lambda a: f"{a:.1f}")
  new_df["2E"]=(df["D2E"]*100).apply(lambda a: f"{a:.1f}")
  new_df["3E"]=(df["D3E"]*100).apply(lambda a: f"{a:.1f}")
  new_df["4E"]=(df["D4E"]*100).apply(lambda a: f"{a:.1f}")
  
  multi_index = pd.MultiIndex.from_tuples([
    ('', 'Data'), 
    ('Index Time(s)', 'Plain'),  
    ('Index Time(s)', '$d=2$'), 
    ('Index Time(s)', '$d=3$'), 
    ('Index Time(s)', '$d=4$'), 
    ('$\epsilon$(\%)', 'Plain'), 
    ('$\epsilon$(\%)', '$d=2$'),
    ('$\epsilon$(\%)', '$d=3$'),
    ('$\epsilon$(\%)', '$d=4$'),
  ])
  new_df.columns=multi_index
  with open(outfile,'w') as tf:
    tf.write(new_df.to_latex(index=False, column_format='l|rrrr|rrrr', escape=False))
  
if __name__ == '__main__':
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp5.csv')
  outfile=f"{CURRENT_DIR}/../figs_and_tables/exp5_table.tex"
  draw_ADO_d_table(df, outfile)
