from graph import graphs
from graph import GRAPH_DIR
import os
import subprocess
from Experiment1 import test_cluster_BFS


def experiment2():
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  LOG_DIR= f"{CURRENT_DIR}/../log/exp2"
  os.makedirs(LOG_DIR, exist_ok=True)
  tests=[f"cluster_BFS_64_{d}" for d in [2,3,4,5,6]]
  for test in tests:
    test_cluster_BFS(test, CURRENT_DIR, LOG_DIR)

if __name__ == '__main__':
  experiment2()