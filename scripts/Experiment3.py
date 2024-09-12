from graph import GRAPH_DIR
import os
import subprocess

def test_ADO(tests, limits, g, CURRENT_DIR, LOG_DIR):
  ground_truth = f"{CURRENT_DIR}/../data/ground_truth/{g}_sym.txt"
  for test in tests:
    print(f"running {test}")
    test_file = f"{CURRENT_DIR}/../build/benchmark/{test}"
    OUT_DIR = f"{LOG_DIR}/{test}"
    os.makedirs(OUT_DIR, exist_ok=True)
    cmd = f"numactl -i all {test_file} {GRAPH_DIR}/{g}_sym.bin -k {limits} -v {ground_truth} >> {OUT_DIR}/{g}.txt"
    subprocess.call(cmd, shell=True)

def experiment3():
  graphs = ["in_2004","uk-2002","socfb-uci-uni"]
  CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
  LOG_DIR= f"{CURRENT_DIR}/../log/exp3"
  os.makedirs(LOG_DIR, exist_ok=True)
  tests=[f"ADO_cluster_{w}_2" for w in [64,32,16,8]]
  tests.append("ADO_base")
  for limits in [256,512,1024,2048,4096]:
    print(f"----ADO memory limits: {limits} bytes per vertex----")
    _LOG_DIR=f"{LOG_DIR}/limit_{limits}"
    for g in graphs:
      print(f"Graph: {g}")
      test_ADO(tests, limits, g, CURRENT_DIR, _LOG_DIR)

if __name__ == '__main__':
  experiment3()