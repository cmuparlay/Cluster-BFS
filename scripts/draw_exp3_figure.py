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

def draw_figure(data,m_sizes, outfile):
    fig, ax = plt.subplots(1,3, figsize=(11.5,2))
    plt.subplots_adjust(left=None, bottom=None, right=None, top=None, wspace=0.5, hspace=0.3)
    ax = ax.flatten()
    ax2 = []
    for j in range(len(ax)):
        ax2.append(ax[j].twinx())
    i = 0
    graphs=["in_2004","uk-2002","socfb-uci-uni"]
    algorithms_map={"Base":"for plain","CC64":"with w=64","CC32":"with w=32","CC16":"with w=16","CC8":"with w=8"}
    sns.color_palette('tab10')
    colors = list(sns.color_palette('tab10'))
    Alg_map={
        "Base":(3.0,colors[3]),
        "CC64":(1.5, colors[0]),
        "CC32":(1.5,colors[1]),
        "CC16":(1.5,colors[2]),
        "CC8":(1.5,colors[4])
    }
    e=r"$\epsilon$"
    m_label=[str(i) for i in m_sizes]
    for g in graphs:
        for a, a_key in algorithms_map.items():
            ax[i].plot(m_sizes, np.array(data[g][f"{a}E"])*100, label=f'{e} {a_key}', linewidth=Alg_map[a][0],color=Alg_map[a][1])
            ax2[i].plot(m_sizes, data[g][f"{a}T"], label=f'time {a_key}', linestyle="dashed",linewidth=Alg_map[a][0],color=Alg_map[a][1])
        ax2[i].set_xscale('log', base = 2)
        ax[i].tick_params(axis = 'y', pad = 0.1, width=0, length=1)
        ax2[i].tick_params(axis = 'y', width=0, length=1, pad=0.1)
        ax[i].set_title(graph_map[g], y=1, fontsize=14)
        ax[i].set_xticks(m_sizes)
        ax[i].set_xticklabels(m_label, fontsize=14)
        yticks=ax[i].get_yticks()
        ax[i].set_yticks(yticks)
        ax[i].set_yticklabels([str(y) for y in yticks],fontsize=14)
        y2ticks = ax2[i].get_yticks()
        ax2[i].set_yticks([y for y in y2ticks if y>0])
        ax2[i].set_yticklabels([str(y) for y in y2ticks if y>0], fontsize=14)
        ax[i].grid(axis='y')
        ax2[i].grid(axis='y', linestyle="dashed")
        ax[i].set_axisbelow(True)
        ax2[i].set_axisbelow(True)
        i+=1
    ax[0].legend(loc='center right',bbox_to_anchor=(-0.22, 0.5), ncol=1, fontsize=14, columnspacing=1, handletextpad=0.5, borderpad=0.2,handlelength=1.3)
    ax2[2].legend(loc='center left',bbox_to_anchor=(1.25, 0.5), ncol=1, fontsize=14, columnspacing=3.9, handletextpad=0.5, borderpad=0.2,handlelength=1.5)
    fig.text(0.08, 0.5, r'$\epsilon$(%)', va='center', rotation='vertical',fontsize = 14)
    fig.text(0.94, 0.5, 'Index Time(s)', va = 'center', rotation=-90, fontsize = 14)
    fig.text(0.43, -0.1, 'Index Size per Vertex(bytes)', va = 'center', fontsize = 14)
    plt.savefig(outfile, bbox_inches='tight')
    
if __name__ == '__main__':
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  os.makedirs(f"{CURRENT_DIR}/../figs_and_tables", exist_ok=True)
  outfile=f"{CURRENT_DIR}/../figs_and_tables/exp3_figure.pdf"
  df = pd.read_csv(f'{CURRENT_DIR}/../result/exp3.csv',index_col=0)
  m_sizes=[256,512,1024,2048,4096]
  # m_sizes=[256,512]
  graphs=["in_2004","uk-2002","socfb-uci-uni"]
  keys=["BaseT","CC64T","CC32T","CC16T","CC8T","BaseE","CC64E","CC32E","CC16E","CC8E"]
  data=dict()
  for g in graphs:
      data[g]=dict()
      for k in keys:
          data[g][k]=[]
  for m in m_sizes:
      for g in graphs:
          for k in keys:
              label = f"{m}{k}"
              data[g][k].append(df[label][g])
  draw_figure(data,m_sizes, outfile)