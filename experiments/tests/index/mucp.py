import itertools

from problems.mucp import MucpBasicModel
from util import irange


def original_instances():
    for instance_size in [20, 30, 60]:
        for days in [2, 4]:
            for symmetry_factor in [2, 3, 4]:
                if symmetry_factor == 4 and instance_size in [20, 30]:
                    continue

                for i in range(20):
                    yield f'{instance_size}_{days*24}_1_3_{symmetry_factor}_0_0_{i+1}.txt'


MODELS = [
    (lambda model: model.add_identical_units_orbitope_constraints(activation_handler=True), 'Act'),
    (lambda model: None, 'Default'),
    (lambda model: model.add_start_up_shut_down_ineqs(), 'Ineq'),
    (lambda model: model.scip.setIntParam("misc/usesymmetry", 0), 'No-Sym'),
]

ORIGINAL_INSTANCES = list(itertools.product(original_instances(), MODELS))
