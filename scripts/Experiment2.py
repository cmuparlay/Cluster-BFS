import os
from Experiment1 import test_cluster_BFS
from data_collection import collect_exp2


def experiment2():
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  LOG_DIR= f"{CURRENT_DIR}/../log/exp2"
  os.makedirs(LOG_DIR, exist_ok=True)
  tests=[f"cluster_BFS_{d}" for d in [2,3,4,5,6]]
  for test in tests:
    test_cluster_BFS(test, CURRENT_DIR, LOG_DIR)

if __name__ == '__main__':
  experiment2()
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  OUT_DIR=f"{CURRENT_DIR}/../result"
  os.makedirs(OUT_DIR, exist_ok=True)
  collect_exp2()