import os
from Experiment3 import test_ADO
from graph import graphs

def experiment4():
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  LOG_DIR= f"{CURRENT_DIR}/../log/exp4"
  os.makedirs(LOG_DIR, exist_ok=True)
  tests=[f"ADO_cluster_{w}_2" for w in [64,32,16,8]]
  tests.append("ADO_base")
  limit = 1024
  print(f"----ADO memory limits: {limit} bytes per vertex----")
  for g in graphs:
    print(f"Graph: {g}")
    test_ADO(tests, limit, g, CURRENT_DIR, LOG_DIR)

if __name__ == '__main__':
  experiment4()