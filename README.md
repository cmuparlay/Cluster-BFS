## Cluster-BFS
This repository contains code for our paper "Parallel Cluster-BFS and Applications to Shortest Paths". Letong Wang, Guy Blelloch, Yan Gu and Yihan Sun.

The project is public available on Zenodo: https://doi.org/10.5281/zenodo.13758048

### Requirements

- Multi-processor linux machine (tested on Rocky Linux 8)
- gcc or clang with C++17 features support (tested with gcc 12 and clang 14)
- CMake 3.16+ (used to compile)
- python3 (used to run scripts, version >= 3.7)
  - pandas (used to collect experiment data)
  - numpy (used to collect experiment data)
  - matplotlib (used to draw figures)
  - seaborn (used to draw figures) 
- We use <a href="https://github.com/cmuparlay/parlaylib">parlaylib</a> for fork-join parallelism. It's provided in our repository `include/parlay`.
- We use <a href="https://github.com/abseil/abseil-cpp.git">abseil</a> library for hash_map.  It's provided as a submodular in  `include/abseil-cpp`


### Reproducibility

#### Get Started
Code download: git clone with submodules

```
git clone --recurse-submodules https://github.com/cmuparlay/Cluster-BFS.git
```

#### Scripts
You can simply run `sh ./RunAll.sh` to reproduce the experiments. If you want to run our server, you can comment the 6th line in `scripts/graph.py` and uncomment the 7th line to avoid downloading graphs and using the graphs stored in our server. If you want to run partial of the experiments, you can comments partial commands in `RunAll.sh`.

The script has fours parts:
  - Part 0: Download the graphs

    The downloaded graphs are stored in  `./data/graphs`
    ``` python3 scripts/download.py```

    You can also download graphs manually from this link [graphs](https://pasgal-bs.cs.ucr.edu/bin/).

    We use the `.bin` binary graph format from [GBBS](https://github.com/ParAlg/gbbs).

  - Part 1: Compile the code

    In the main folder `Cluster-BFS`:
    ```mkdir build && cd build```

    ```cmake  -DCMAKE_BUILD_TYPE=Release  ..```

    ```cd benchmark && make -j```

    Explain for the executable files:
      - Akiba_BFS_seq: the baseline AIY algorithm's cluster BFS
      - cluster_BFS_{d}: our cluster BFS with diameter d. If d are not specified, the default values is 2. If the name is end up with "_seq", it is compiled for sequential setting. 
      - ADO_base: an approximate distance oracle (ADO) choosing a single vertex as a landmark
      - ADO_cluster_{w}_{d}: a ADO choosing a cluster of vertices as landmarks, where w and d are the size and diameter of clusters.
  - Part 2: Run Experiments

    I catogarize the whole experiments into five experiment sets. I will briefly introduce what they test and their corresponding tables or figures in the paper.

    - Experiment 1 (takes about 30 hours): 

      Testing the running time (in seconds) of all BFS algorithms. It will generate the data listed in `Table 1` and `Figure 3` in the paper.
    - Experiment 2 (takes about 50 minutes): 
    
      Testing the running time (in seconds) of C-BFS with different cluster diameter `d`.  
      It will generate the data used in `Figure 4`(in main body of paper) and `Table 3` (in Appendix). 
    - Experiment 3 (takes about 4 hours):

      Testing the running time and distortion of our Approximate Distance Oracle (ADO) under different cluster size (from 1 to 64) and index size. It will generate the data used in `Figure 5` in the paper.
    - Experiment 4 (takes about 5 hours): 

      Testing the running time and distortion of our ADO under different cluster size (from 1 to 64) with the same index size limitation (1024 Bytes per vertex). It will generate the data used in `Table 2` in the main body of paper and `Table 5` in the Appendix`.
    - Experiment 5 (takes about 3 hours): 

      Testing the performance of our ADO with different cluster diameters. It will generate the data listed in `Table 4` in the Appendix.
    
    For each experiment set, the script will first run the test, whose output is stored in the `./log` folder. Then, the script collects required data in `./log` in a `.csv` format and stores in `./result` folder. Finally, the script will draw corresponding figures (in .pdf format) and tables (in .tex format) using the data from `./result`, and store the figures and tables in `./figs_and_tables`. 

  - Part 3: Generate Report

    The latex file `report.tex` will generate a report to show the figures and tables in `figs_and_tables`.

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
