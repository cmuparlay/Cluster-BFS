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



def draw_BFS_d(df,outfile):
    # Share x axis
    fig, ax = plt.subplots(1,1, figsize=(3,2), sharex=True)
    # adjust magin between axis, and axis to figure edges
    plt.subplots_adjust(left=None, bottom=None, right=None, top=None, wspace=0.16, hspace=0.25)
    x=[2,3,4,5,6]
    # maps for style
    colors = list(sns.color_palette('tab10'))
    graphs = ["DBLP","soc-LiveJournal1","hollywood_2009","socfb-uci-uni","com-orkut","indochina","uk-2002","arabic","twitter","sd_arc"
    ]
    i =0
    for g in graphs:
        ax.plot(x, df.loc[g]/df.loc[g][0],marker="o", markersize = 8, label=graph_map[g], alpha=0.5, linewidth=2.0)
        i+=1
    # ax.set_yscale('log', base = 2)
    ax.tick_params(axis = 'y', pad = 0.1)
    # show grid background and mark major and minor with different style
    ax.set_xticks(x)
    ax.set_xticklabels(["2","3","4","5","6"])
    ax.grid(which='major', color='#CCCCCC', linewidth=1.1)
    ax.grid(which='minor', color='#CCCCCC', linestyle=':')
    # show the legend
    ax.legend(loc='center left',bbox_to_anchor=(1.0,0.5), ncol=2, fontsize=14,columnspacing=0.8) 
    # add common x/y-label, labelpad adjust the margin between label to the figure
    ax.set_xlabel("Cluster Diameter (d)", fontsize=14, labelpad=1)
    ax.set_ylabel("Relative Time", fontsize=14, labelpad=1)
    # ------------------New added -------------------------------
    # save figure
    plt.savefig(outfile, bbox_inches='tight')
    # -----------------------------------------------------------


if __name__ == "__main__":
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  outfile=f"{CURRENT_DIR}/../figs_and_tables/exp2_figure.pdf"
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp2.csv',index_col=0)
  draw_BFS_d(df,outfile)