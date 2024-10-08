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
        return f"{num:.2f}"
    else:
        return f"{num:.3f}"
def geo_mean(iterable):
    a = np.array(iterable)
    return a.prod()**(1.0/len(a))


def draw_table_BFS_d(df,outfile):
  new_df = pd.DataFrame()
  new_df['Data']=df['Graph'].apply(lambda a: graph_map[a])
  new_df["$d=2$"]=(df['2']/10).apply(format_digit)
  new_df["$d=3$"]=(df['3']/10).apply(format_digit)
  new_df["$d=4$"]=(df["4"]/10).apply(format_digit)
  new_df["$d=5$"]=(df["5"]/10).apply(format_digit)
  new_df["$d=6$"]=(df["6"]/10).apply(format_digit)
  GeoMean=dict()
  GeoMean['Data']="GeoMean"
  GeoMean['$d=2$']=format_digit(geo_mean(df['2']/10))
  GeoMean['$d=3$']=format_digit(geo_mean(df['3']/10))
  GeoMean['$d=4$']=format_digit(geo_mean(df['4']/10))
  GeoMean['$d=5$']=format_digit(geo_mean(df['5']/10))
  GeoMean['$d=6$']=format_digit(geo_mean(df['6']/10))

#   new_df=new_df.append(GeoMean,ignore_index=True)
  GeoMean_row = pd.DataFrame([GeoMean])
  new_df = pd.concat([new_df, GeoMean_row], ignore_index=True)
  with open(outfile,'w') as tf:
    tf.write(new_df.to_latex(index=False, column_format='l|rrrrrr', escape=False))
if __name__ == '__main__':
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  outfile=f"{CURRENT_DIR}/../figs_and_tables/exp2_table.tex"
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp2.csv')
  draw_table_BFS_d(df,outfile)

  
  
  