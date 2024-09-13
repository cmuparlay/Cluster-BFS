import os
from Experiment3 import test_ADO
from graph import graphs
from data_collection import collect_exp5

def experiment5():
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  LOG_DIR= f"{CURRENT_DIR}/../log/exp5"
  os.makedirs(LOG_DIR, exist_ok=True)
  tests=[f"ADO_cluster_64_{d}" for d in [2,3,4]]
  tests.append("ADO_base")
  limit = 1024
  print(f"----ADO memory limits: {limit} bytes per vertex----")
  for g in graphs:
    print(f"Graph: {g}")
    test_ADO(tests, limit, g, CURRENT_DIR, LOG_DIR)

if __name__ == '__main__':
  experiment5()
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  OUT_DIR=f"{CURRENT_DIR}/../result"
  os.makedirs(OUT_DIR, exist_ok=True)
  collect_exp5()