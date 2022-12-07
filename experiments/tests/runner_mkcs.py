import os
import sys
import time
from collections.abc import Callable

from graph.edgelist import GraphEdgeListNeighbors
from index.mkcs import SELECTED_INSTANCES
from problems.mkcs import MkcsModel, MkcsInstance

COLOR02_INSTANCE_DIR = 'data/Color02'

def test_color02_instance(graph_file: str, k: int, subsym_handler: Callable[[MkcsModel], None], model_name: str):
    print(f'{graph_file}__{k}__{model_name}')

    graph = GraphEdgeListNeighbors.read_from_col_file(os.path.join(COLOR02_INSTANCE_DIR, graph_file))
    instance = MkcsInstance(graph, k)

    start_time = time.time()
    model = MkcsModel(instance)

    model.build(memory_limit=10_000)  # 10 GB
    print(f'Initializing model took: {time.time()-start_time} seconds.')

    start_time = time.time()
    subsym_handler(model)
    print(f'Initializing subsym handler took: {time.time()-start_time} seconds.')

    scip = model.scip
    model.scip.setIntParam('display/verblevel', 0)
    scip.setRealParam('limits/time', 7200)
    start_time = time.time()
    scip.optimize()
    print(f'Solving took: {time.time()-start_time} seconds.')
    scip.writeStatistics(f'out/mkcs/results/{graph_file}__{k}__{model_name}.stats')


if __name__ == '__main__':
    instance, k, model = SELECTED_INSTANCES[int(sys.argv[1])]
    test_color02_instance(instance, k, *model)
