from graph import graphs
from graph import GRAPH_DIR
import os
import subprocess

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_DIR= f"{CURRENT_DIR}/../log/exp1"
os.makedirs(LOG_DIR, exist_ok=True)


def test_cluster_BFS(test):
  print(f"Testing {test}")
  test_file = f"{CURRENT_DIR}/../build/benchmark/{test}"
  OUT_DIR = f"{LOG_DIR}/{test}"
  os.makedirs(OUT_DIR, exist_ok=True)
  numa = "" if test[-3:]=="seq" else "numactl -i all"
  for g in graphs:
    print(f"  Graph: {g}")
    cmd = f"{numa} {test_file} {GRAPH_DIR}/{g}_sym.bin >> {OUT_DIR}/{g}.txt"
    subprocess.call(cmd, shell=True)

if __name__ == '__main__':
  tests=["sequential_BFS_seq", "ligra_BFS_seq", "ligra_BFS", "Akiba_BFS_seq", "cluster_BFS", "cluster_BFS_seq"]
  for test in tests:
    test_cluster_BFS(test)

