import os
import sys
import subprocess
import re
import pandas as pd
import numpy as np
from graph import graphs
from graph import GRAPH_DIR

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

def collect_data(file_in, key_words):
    f = open(file_in,'r')
    res = f.read()
    f.close()
    data_lines = re.findall(f"{key_words}.*", res)
    data = list(map(lambda x: eval(x.split(" ")[-1]), data_lines))
    return data

def collect_exp1():
    print("Collecting data in Experiment 1")
    LOG_DIR= f"{CURRENT_DIR}/../log/exp1"
    OUT_FILE=f"{CURRENT_DIR}/../result/exp1.csv"
    data=dict()
    data["Dataset"]=graphs
    data["n"]=[]
    data["m"]=[]
    tests=["sequential_BFS_seq", "ligra_BFS_seq", "ligra_BFS", "Akiba_BFS_seq", "cluster_BFS", "cluster_BFS_seq"]
    for t in tests:
        data[t]=[]
    for g in graphs:
        log_file = f"{LOG_DIR}/{tests[0]}/{g}.txt"
        data["n"].append(collect_data(log_file,"num vertices")[0])
        data["m"].append(collect_data(log_file,"num edges")[0])
        for t in tests:
            log_file = f"{LOG_DIR}/{t}/{g}.txt"
            data[t].append(collect_data(log_file,"average")[0])
    for t in ["Akiba_BFS_seq", "cluster_BFS", "cluster_BFS_seq"]:
        data[t]=np.array(data[t])/10.0
    df = pd.DataFrame.from_dict(data)
    df.to_csv(OUT_FILE, index=False)
def collect_exp2():
    print("Collecting data in Experiment 2")
    LOG_DIR= f"{CURRENT_DIR}/../log/exp2"
    OUT_FILE=f"{CURRENT_DIR}/../result/exp2.csv"
    diameters=[2,3,4,5,6]
    data=dict()
    data["Graph"]=graphs
    for d in diameters:
        data[d]=[]
        for g in graphs:
            log_file=f"{LOG_DIR}/cluster_BFS_{d}/{g}.txt"
            data[d].append(collect_data(log_file,"average cluster BFS time:")[0])
    df = pd.DataFrame.from_dict(data)
    df.to_csv(OUT_FILE, index=False)

def collect_exp3():
    print("Collecting data in Experiment 3")
    LOG_DIR= f"{CURRENT_DIR}/../log/exp3"
    OUT_FILE=f"{CURRENT_DIR}/../result/exp3.csv"
    exp3_graphs = ["in_2004","uk-2002","socfb-uci-uni"]
    data=dict()
    data["Dataset"]=exp3_graphs
    tests={"ADO_base": "Base",
        "ADO_cluster_64_2":"CC64",
        "ADO_cluster_32_2":"CC32",
        "ADO_cluster_16_2":"CC16",
        "ADO_cluster_8_2":"CC8",
    }
    for limits in [256,512,1024,2048,4096]:
        for key,val in tests.items():
            data[f"{limits}{val}T"]=[]
            for g in exp3_graphs:
                log_file = f"{LOG_DIR}/limit_{limits}/{key}/{g}.txt"
                data[f"{limits}{val}T"].append(collect_data(log_file,"Average time for construct index")[0])
    for limits in [256,512,1024,2048,4096]:
        for key,val in tests.items():
            data[f"{limits}{val}E"]=[]
            for g in exp3_graphs:
                log_file = f"{LOG_DIR}/limit_{limits}/{key}/{g}.txt"
                data[f"{limits}{val}E"].append(collect_data(log_file,"distortion:")[0])
    df = pd.DataFrame.from_dict(data)
    df.to_csv(OUT_FILE, index=False)

def collect_exp4():
    print("Collecting data in Experiment 4")
    LOG_DIR= f"{CURRENT_DIR}/../log/exp4"
    OUT_FILE=f"{CURRENT_DIR}/../result/exp4.csv"
    data=dict()
    data["Data"]=graphs
    key_words = {
        "T": "Average time for construct index",
        "E": "distortion:",
        "Q": "query time"
    }
    tests={"ADO_base": "Base",
        "ADO_cluster_64_2":"CC64",
        "ADO_cluster_32_2":"CC32",
        "ADO_cluster_16_2":"CC16",
        "ADO_cluster_8_2":"CC8",
    }
    for key, words in key_words.items():
        for test, alg in tests.items():
            data[f"{alg}{key}"]=[]
            for g in graphs:
                log_file = f"{LOG_DIR}/{test}/{g}.txt"
                data[f"{alg}{key}"].append(collect_data(log_file, words)[0])
    df = pd.DataFrame.from_dict(data)
    df.to_csv(OUT_FILE, index=False)

def collect_exp5():
    print("Collecting data in Experiment 5")
    LOG_DIR= f"{CURRENT_DIR}/../log/exp5"
    OUT_FILE=f"{CURRENT_DIR}/../result/exp5.csv"
    data=dict()
    data["Data"]=graphs
    key_words = {
        "T": "Average time for construct index",
        "E": "distortion:",
    }
    tests = {
        "ADO_base":"Base",
        "ADO_cluster_64_2":"D2",
        "ADO_cluster_64_3":"D3",
        "ADO_cluster_64_4":"D4"
    }
    for key, word in key_words.items():
        for test,alg in tests.items():
            data[f"{alg}{key}"]=[]
            for g in graphs:
                log_file=f"{LOG_DIR}/{test}/{g}.txt"
                data[f"{alg}{key}"].append(collect_data(log_file,word)[0])
    df = pd.DataFrame.from_dict(data)
    df.to_csv(OUT_FILE, index=False)

if __name__ == '__main__':
    OUT_DIR=f"{CURRENT_DIR}/../result"
    os.makedirs(OUT_DIR, exist_ok=True)
    collect_exp1()
    collect_exp2()
    collect_exp3()
    collect_exp4()
    collect_exp5()
