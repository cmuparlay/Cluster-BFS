import os
import subprocess

from graph import graphs
from graph import website

from graph import GRAPH_DIR


if __name__ == '__main__':
  if GRAPH_DIR == "/data/graphs/bin":
    print("Running on our server. Don't need to download the graphs.")
  else:
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
  