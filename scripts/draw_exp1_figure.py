import matplotlib.pyplot as plt    # used for drawing figures
import numpy as np                 # for linear algebra operations
import seaborn as sns              # the others are for format details
import pandas as pd
from matplotlib.ticker import MaxNLocator
import matplotlib.font_manager as font_manager
from matplotlib.ticker import FuncFormatter
import itertools
import os
from graph import graph_map



def format_digit2(num):
    if pd.isna(num):
        return ''
    if num > 100:
        return f"{int(num)}"
    elif 10 <= num < 100:
        return f"{num:.0f}"
    else:
        return f"{num:.1f}"

def geo_mean(iterable):
    a = np.array(iterable)
    return a.prod()**(1.0/len(a))

def draw_speedup(df, outfile):
    # Share x axis
    fig, ax = plt.subplots(1,1, figsize=(22,1.5), sharex=True)
    # adjust magin between axis, and axis to figure edges
    plt.subplots_adjust(left=None, bottom=None, right=None, top=None, wspace=0.16, hspace=0.25)
    y1 = list(df["sequential_BFS_seq"]/df["Akiba_BFS_seq"])
    y1.append(geo_mean(df["sequential_BFS_seq"]/df["Akiba_BFS_seq"]))
    y2 = list(df["sequential_BFS_seq"]/df["ligra_BFS"])
    y2.append(geo_mean(df["sequential_BFS_seq"]/df["ligra_BFS"]))
    y3 = list(df["sequential_BFS_seq"]/df["cluster_BFS"])
    y3.append(geo_mean(df["sequential_BFS_seq"]/df["cluster_BFS"]))
    x_label= [graph_map[val] for val in df['Dataset']]
    x_label.append("Mean")
    x1 = [i*1.3 for i in range(len(y1))]
    x2=[i+0.3 for i in x1]
    x3=[i+0.3 for i in x2]
    x_ticks=[i+0.3 for i in x1]
    i =0
    # set x/y-axis to log2-scaled
    bar1=ax.bar(x1,y1, label="AIY",width = 0.3)
    bar2=ax.bar(x2,y2, label="Ligra", width = 0.3)
    bar3=ax.bar(x3,y3, label="Parallel C-BFS", width = 0.3)
    for rect in bar1+bar2+bar3:
        height=rect.get_height()
        plt.text(rect.get_x() + rect.get_width() / 2.0, height, format_digit2(height), ha='center', va='bottom',rotation=0,fontsize=14)
        i+=1
    ax.set_xticks(x_ticks)
    ax.set_xticklabels(x_label, fontsize=14, rotation=60)
    ax.set_yscale('log', base = 2)
    ax.set_yticks([10, 100, 1000])
    ax.set_yticklabels(["10", "100", "1000"], fontsize=14)
    ax.tick_params(axis = 'y', pad = 0.1)
    # show the legend
    ax.legend(loc='lower center',bbox_to_anchor=(0.5,1.0), ncol=3, fontsize=14,columnspacing=0.8) 
    # add common x/y-label, labelpad adjust the margin between label to the figure
#     ax.set_xlabel("number of processors", fontsize=14, labelpad=1)
    ax.set_ylabel("speedup over \nsequential BFS", fontsize=14, labelpad=1)
    # ------------------New added -------------------------------
    # save figure
    plt.xlim(-0.5, x1[-1]+1)
    plt.ylim(1,8000)
    plt.savefig(outfile, bbox_inches='tight')
    # -----------------------------------------------------------


if __name__=="__main__":
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  outfile=f"{CURRENT_DIR}/../figs_and_tables/exp1_figure.pdf"
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp1.csv')
  draw_speedup(df, outfile)