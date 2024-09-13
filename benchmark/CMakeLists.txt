# Benchmarks for Cluster-BFS
#
# set(NUMA_COMMAND numactl -i all)

function(add_benchmark_parallel NAME)
  add_executable(${NAME} ${NAME}.cpp)
  target_link_libraries(${NAME} PRIVATE parlay absl::flat_hash_map)
endfunction()

function(add_benchmark_sequential NAME)
  add_executable(${NAME}_seq ${NAME}.cpp)
  target_link_libraries(${NAME}_seq PRIVATE parlay absl::flat_hash_map)
  target_compile_definitions(${NAME}_seq PRIVATE PARLAY_SEQUENTIAL=1)
endfunction()

function(add_benchmark_Radius NAME)
  foreach(VALUE 2 3 4 5 6)
    add_executable(${NAME}_${VALUE} ${NAME}.cpp)
    target_compile_definitions(${NAME}_${VALUE} PRIVATE Radius=${VALUE})
    target_link_libraries(${NAME}_${VALUE} PRIVATE parlay)
  endforeach()
endfunction()

# function(add_benchmark_multi NAME)
#   foreach(VALUE 2048 1024 512 256 128 64 32 16 1 0)
#       add_executable(${NAME}_${VALUE} ${NAME}.cpp)
#       target_compile_definitions(${NAME}_${VALUE} PRIVATE MY_TEMPLATE_VALUE=${VALUE})
#       target_link_libraries(${NAME}_${VALUE} PRIVATE parlay)
#   endforeach()
# endfunction()

function(add_benchmark_ADO NAME)
  foreach(LABEL 64 32 16 8)
    foreach(VALUE 2 3 4 5 6)
      add_executable(${NAME}_${LABEL}_${VALUE} ${NAME}.cpp)
      target_compile_definitions(${NAME}_${LABEL}_${VALUE} PRIVATE Radius=${VALUE} label=uint${LABEL}_t)
      target_link_libraries(${NAME}_${LABEL}_${VALUE} PRIVATE parlay absl::flat_hash_map)
    endforeach()
  endforeach()
endfunction()




add_benchmark_sequential(sequential_BFS)
add_benchmark_sequential(ligra_BFS)
add_benchmark_sequential(Akiba_BFS)
add_benchmark_sequential(cluster_BFS)
add_benchmark_parallel(ligra_BFS)
add_benchmark_parallel(cluster_BFS)
add_benchmark_Radius(cluster_BFS)
add_benchmark_parallel(ADO_base)
add_benchmark_ADO(ADO_cluster)
# add_benchmark_multi(construct_index)