#!/bin/bash

######### Part 0: Download the graphs #########
# If you are using our server, the graph is already in disk. You need to uncomment the 7th line and comment 6th line in scripts/graph.py to change the GRAPH_DIR
echo "Preparing for Graphs"
python3 scripts/download.py

######### Part 1: Compile the code #########
echo "Compiling"
mkdir build
cd build
cmake  -DCMAKE_BUILD_TYPE=Release ..
cd benchmark && make -j
cd ../../

######## Part 2: Run Experiments 1-5 #########
#----------Experiment 1----------
echo "Start Experiment1: $(date)"
python3 scripts/Experiment1.py
echo "Drawing figures and tables"
python3 scripts/draw_exp1_figure.py
python3 scripts/draw_exp1_table.py
#----------Experiment 2----------
echo "Start Experiment2: $(date)"
python3 scripts/Experiment2.py
echo "Drawing figures and tables"
python3 scripts/draw_exp2_figure.py
python3 scripts/draw_exp2_table.py
#----------Experiment 3----------
echo "Start Experiment3: $(date)"
python3 scripts/Experiment3.py
echo "Drawing figures and tables"
python3 scripts/draw_exp3_figure.py
#----------Experiment 4----------
echo "Start Experiment4: $(date)"
python3 scripts/Experiment4.py
echo "Drawing figures and tables"
python3 scripts/draw_exp4_tables.py
#----------Experiment 5----------
echo "Start Experiment5: $(date)"
python3 scripts/Experiment5.py
echo "Drawing figures and tables"
python3 scripts/draw_exp5_table.py
echo "Finish Experiment5: $(date)"


# ######## Part 3: Generating Report #########
echo "Generating Report"
pdflatex report.tex
pdflatex report.tex
rm report.log report.aux

