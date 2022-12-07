import sys
import time
from collections.abc import Callable

from index.mkp import SELECTED_INSTANCES, MkpBenchmarkSpec
from problems.multipleknapsack import MultipleKnapsackModel, MultipleKnapsackInstance


def test_instance(spec: MkpBenchmarkSpec, subsym_handler: Callable[[MultipleKnapsackModel], None], model_name: str):
    name = f'{spec.item_class.name}__K{spec.nknapsacks}__I{spec.nitems}__F{spec.symmetry_factor}__E{spec.equal_profit}__{spec.index}'
    print(f'{name}__{model_name}')
    with open(f'data/mkp/{name}.json', 'r') as f:
        mkpi = MultipleKnapsackInstance.from_json(f.read())

    start_time = time.time()
    model = MultipleKnapsackModel(mkpi)

    model.build(memory_limit=10_000)  # 10 GB
    if model_name != 'No-Int-Sym':
        model.add_item_orbitopes()
    print(f'Initializing basic model took: {time.time()-start_time} seconds.')

    start_time = time.time()
    subsym_handler(model)
    print(f'Initializing subsym handler took: {time.time()-start_time} seconds.')

    scip = model.scip
    model.scip.setIntParam('display/verblevel', 0)
    scip.setRealParam('limits/time', 3600)
    start_time = time.time()
    scip.optimize()
    print(f'Solving took: {time.time()-start_time} seconds.')
    scip.writeStatistics(f'out/mkp/results/{name}__{model_name}.stats')


if __name__ == '__main__':
    instance, model = SELECTED_INSTANCES[int(sys.argv[1])]
    test_instance(instance, *model)
