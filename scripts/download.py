import os
import subprocess

from graph import graphs
from graph import website

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

GRAPH_DIR = f"{CURRENT_DIR}/../data/graphs"


if __name__ == '__main__':
  print("Downloading graphs")
  os.makedirs(GRAPH_DIR, exist_ok=True)
  for g in graphs:
    graph_file = f"{GRAPH_DIR}/{g}_sym.bin"
    if os.path.exists(graph_file):
      print(f"{g} exists!")
      continue
    url = f"{website}{g}_sym.bin"
    print(f"Downloading {g} from {url}")
    subprocess.call(f"wget -O {graph_file} {url}", shell=True)
    print(f"Successfully downloaded {g}")
  