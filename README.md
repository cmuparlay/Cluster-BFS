## Cluster-BFS
This repository contains code for our paper "Parallel Cluster-BFS and Applications to Shortest Paths". Letong Wang, Guy Blelloch, Yan Gu and Yihan Sun.

### Requirements

- Multi-processor linux machine (tested on CentOS 8 and MacOS 13)
- gcc or clang with C++17 features support (tested with gcc 12 and clang 14)
- cmake 3.14+ (used to compile)
- python3 (used to run scripts, version >= 3.7)
  - pandas (used to collect experiment data)
  - numpy (used to collect experiment data)
  - matplotlib (used to draw figures)
  - seaborn (used to draw figures) 
- We use <a href="https://github.com/cmuparlay/parlaylib">parlaylib</a> for fork-join parallelism. It's provided in our repository `include/parlay`.


### Reproducibility

You can simply run `python3 RunAll.py` to reproduce the experiments. If you want to run our server, you can change the 6th line in `scripts/graph.py` to `GRAPH_DIR="/data/graphs/bin"` to avoid download graphs but using the ones stored in our server. Or you can run scripts step by step as follows.
#### Step Zero: Download Graphs and Compile the code
- Download the all graphs to `./data`
  ``` python3 scripts/download.py```
These command will download the graphs to `./data/graphs` that are used in this paper. 

You can also download graphs manually from this link [graphs](https://pasgal-bs.cs.ucr.edu/bin/).

We use the `.bin` binary graph format from [GBBS](https://github.com/ParAlg/gbbs).

- Compile the code
  In the main folder `Cluster-BFS`:
  ```mkdir build && cd build```

  ```cmake  -DCMAKE_BUILD_TYPE=Release  ..```
  
  ```cd benchmark && make -j```
  
  
  The executable files:
  - Akiba_BFS_seq: the baseline AIY algorithm's cluster BFS
  - cluster_BFS_{w}_{d}: our cluster BFS with word size w and diameter d. If w and d are not specified, the default values are 64 and 2. If the name is end up with "_seq", it is compiled for sequential setting. 


#### Step One: Run Experiments
The scripts of running experiment are under `./scripts` folder.
- Experiment 1: Testing the running time (in seconds) of all BFS algorithms. It will generate the data listed in `Table 1` and `Figure 3` in the paper.

  ``` python3 scripts/Experiment1.py ```
  
  The output files are stored in `./log/exp1`.
- Experiment 2: Testing the running time (in seconds) of C-BFS with different cluster diameter `d`.  
 It will generate the data used in `Figure 4`(in main body of paper) and `Table 3` (in Appendix). 

  ```python3 scripts/Experiment2.py```

  The output files are stored in `./log/exp2`.
- Experiment 3: Testing the running time and distortion of our Approximate Distance Oracle (ADO) under different cluster size (from 1 to 64) and index size. It will generate the data used in `Figure 5` in the paper.

  ```python3 scripts/Experiment3.py```

  The output files are stored in `./log/exp3`.
- Experiment 4: Testing the running time and distortion of our ADO under different cluster size (from 1 to 64) with the same index size limitation (1024 Bytes per vertex). It will generate the data used in `Table 2` in the main body of paper and `Table 5` in the Appendix`.

  ```python3 scripts/Experiment4.py```
  
  The output files are stored in `./log/exp4`.
- Experiment 5:  Testing the performance of our ADO with different cluster diameters. It will generate the data listed in `Table 4` in the Appendix.

  ```python3 scripts/Experiment5.py```

  The output files are stored in `./log/exp5`.
- Experiment 6: Testing the performance of our Parallel Pruned Landmark Labeling and the sequential baseline algorithm. It will generate the data used in `Table 6` in the Appendix.

  ```python3 scripts/Experiment6.py```

  The output files are stored in `./log/exp6`.

#### Step Two: Collect Data
It will collect the data in `./log` folder, and generate the `.csv` format files in `./result`

```python3 scripts/data_collection.py```


#### Step Three: Draw Figures / Generate Tables
It will use the data in `./result` folder to generate figures of `.pdf` format in `./figures`.
Note that since Figure 1 is essentially a table, it is not in `./figures`. But `Figure1.cvs` is in `./result` folder among with `Table3.csv` and `Table4.csv`

#### Step Four: Show all the results in a PDF file.


#### Our Machine Information
- CPU: 4x Intel(R) Xeon(R) Gold 6252 CPU @ 2.10GHz
- Physical CPU cores: 96
- Threads per core: 2
- NUMA nodes: 4
  - node0 CPUs: 0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188
  - node1 CPUs: 1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,65,69,73,77,81,85,89,93,97,101,105,109,113,117,121,125,129,133,137,141,145,149,153,157,161,165,169,173,177,181,185,189
  - node2 CPUs: 2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,66,70,74,78,82,86,90,94,98,102,106,110,114,118,122,126,130,134,138,142,146,150,154,158,162,166,170,174,178,182,186,190
  - node3 CPUs: 3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,63,67,71,75,79,83,87,91,95,99,103,107,111,115,119,123,127,131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191
- Memory: 1510 GB
- Operation system: CentOS Stream 8

Our scalability test script is written based on the above Machine information. When we run with fewer cores, try to run the code on the same socket.
