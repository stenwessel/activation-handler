import sys
import time
from collections.abc import Callable

from index.mucp import ORIGINAL_INSTANCES
from problems.mucp import MucpBasicModel, MucpInstance


def test_mucp_instance(file_name: str, sym_handler: Callable[[MucpBasicModel], None], model_name: str):
    print(f'{file_name}__{model_name}')

    with open(f'data/mucp/{file_name}', 'r') as f:
        instance = MucpInstance.read_orig(f.read())

    start_time = time.time()
    model = MucpBasicModel(instance)
    model.build(memory_limit=10_000)
    print(f'Initializing basic model took: {time.time()-start_time} seconds.')

    start_time = time.time()
    sym_handler(model)
    print(f'Initializing sym handler took: {time.time()-start_time} seconds.')

    scip = model.scip
    model.scip.setIntParam('display/verblevel', 0)
    scip.setRealParam('limits/time', 3600)

    start_time = time.time()
    scip.optimize()
    print(f'Solving took: {time.time()-start_time} seconds.')
    scip.writeStatistics(f'out/mucp/results/{number}__{model_name}.stats')

    assert scip.getStatus() == 'optimal'


if __name__ == '__main__':
    number, model = ORIGINAL_INSTANCES[int(sys.argv[1])]
    test_mucp_instance(number, *model)
