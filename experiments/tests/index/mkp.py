import itertools
import math
from dataclasses import dataclass
from enum import Enum

from numpy.random import default_rng, Generator

from problems.multipleknapsack import MultipleKnapsackInstance, Item, Weight, Knapsack, Profit, ItemIndex


class ItemClass(Enum):
    UNCORRELATED = 0
    WEAKLY_CORRELATED = 1
    STRONGLY_CORROLATED = 2
    MULTIPLE_SUBSET_SUM = 3


def generate_profit(
        rng: Generator,
        weight: Weight,
        item_min: int,
        item_max: int,
        item_class: ItemClass,
) -> Profit:
    profit = None
    match item_class:
        case ItemClass.UNCORRELATED:
            profit = Profit(int(rng.integers(item_min, item_max, endpoint=True)))

        case ItemClass.WEAKLY_CORRELATED:
            profit = Profit(
                int(rng.integers(max(1, weight - (item_max - item_min) // 10), weight + (item_max - item_min) // 10, endpoint=True)))

        case ItemClass.STRONGLY_CORROLATED:
            profit = Profit(weight + (item_max - item_min) // 10)

        case ItemClass.MULTIPLE_SUBSET_SUM:
            profit = Profit(weight)

    return profit


def generate_instance(
        nitems: int,
        nknapsacks: int,
        seed: int,
        item_min: int = 10,
        item_max: int = 1000,
        item_class: ItemClass = ItemClass.UNCORRELATED,
        symmetry_factor: int = None,
        item_symmetry_equal_profit: bool = True,
) -> MultipleKnapsackInstance:
    # Adapted from (for example, there are others) https://link.springer.com/content/pdf/10.1007/s10479-009-0660-y.pdf
    # n/m ratio from 2 to 10
    rng = default_rng(seed=seed)

    items = []
    count = 0

    if symmetry_factor is None:
        symmetry_factor = nitems

    while count < nitems:
        multiplicity = int(rng.integers(1, nitems//symmetry_factor))
        if count + multiplicity > nitems:
            # Make sure total count is not exceeded
            multiplicity = nitems - count

        weight = Weight(int(rng.integers(item_min, item_max, endpoint=True)))

        if item_symmetry_equal_profit:
            profit = generate_profit(rng, weight, item_min, item_max, item_class)

            items.append(Item(weight, profit, ItemIndex(multiplicity)))
            count += multiplicity
        else:
            profits = [generate_profit(rng, weight, item_min, item_max, item_class) for _ in range(multiplicity)]
            profits = sorted(profits)
            for profit, g in itertools.groupby(profits):
                i = len(list(g))
                items.append(Item(weight, profit, ItemIndex(i)))
                count += i

    weight_sum = sum(item.weight * item.multiplicity for item in items)

    # Exactly distribute half of the item weight sum over the knapsacks
    capacity = Weight(int(math.floor(0.5*weight_sum / nknapsacks)))

    knapsacks = [
        Knapsack(
            capacity=capacity
        )
        for _ in range(nknapsacks)
    ]

    return MultipleKnapsackInstance(knapsacks, items)


@dataclass
class MkpBenchmarkSpec:
    item_class: ItemClass
    nknapsacks: int
    nitems: int
    symmetry_factor: int
    equal_profit: bool
    index: int

def make_test_set():
    for item_class in [ItemClass.STRONGLY_CORROLATED, ItemClass.UNCORRELATED, ItemClass.MULTIPLE_SUBSET_SUM, ItemClass.WEAKLY_CORRELATED]:
        for nbins, nitems in [(30, 60), (12, 48), (15, 75), (10, 60), (10, 100)]:
            for sym_factor in [2, 3, 4, 8]:
                for equal_profit in [True, False]:
                    if not equal_profit and item_class in [ItemClass.STRONGLY_CORROLATED, ItemClass.MULTIPLE_SUBSET_SUM]:
                        continue

                    for i in range(20):
                        yield MkpBenchmarkSpec(item_class, nbins, nitems, sym_factor, equal_profit, i)


TEST_SET = [spec for spec in make_test_set()]


def generate_test_set():
    seed = 1985
    for spec in TEST_SET:
        instance = generate_instance(spec.nitems, spec.nknapsacks, seed,
                                     item_class=spec.item_class,
                                     symmetry_factor=spec.symmetry_factor,
                                     item_symmetry_equal_profit=spec.equal_profit)
        with open(f'../data/mkp/{spec.item_class.name}__K{spec.nknapsacks}__I{spec.nitems}__F{spec.symmetry_factor}__E{spec.equal_profit}__{spec.index}.json', 'w') as f:
            f.write(instance.to_json())

        seed += 1


if __name__ == '__main__':
    generate_test_set()


MODELS = [
    (lambda model: model.add_makespan_suborbitope_constraints(), 'Act'),
    (lambda model: None, 'Default'),
    (lambda model: model.add_makespan_suborbitope_constraints(activation_handler=False), 'Orbitope'),
    (lambda model: model.scip.setIntParam("misc/usesymmetry", 0), 'No-Sym'),
    (lambda model: model.add_subsym_ineqs(consecutive_pairs=True), 'Ineq'),
]

SELECTED_INSTANCES = list(
    itertools.product(
        TEST_SET,
        MODELS,
    )
)
