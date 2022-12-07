"""
Multiple Knapsack problem
"""
import itertools
from dataclasses import dataclass
from typing import Mapping, NewType, Iterator

from dataclasses_json import dataclass_json

from pyscipopt import Model
from pyscipopt.scip import Variable, quicksum
from pyscipopt.symmetry import OrbitopeType

from util import pairwise

KnapsackIndex = NewType('KnapsackIndex', int)
ItemIndex = NewType('ItemIndex', int)
Weight = NewType('Weight', int)
Profit = NewType('Profit', int)


@dataclass_json
@dataclass(frozen=True)
class Knapsack:
    capacity: Weight


@dataclass_json
@dataclass(frozen=True)
class Item:
    weight: Weight
    profit: Profit
    multiplicity: ItemIndex = 1


@dataclass_json
@dataclass(frozen=True)
class MultipleKnapsackInstance:
    knapsacks: list[Knapsack]
    unique_items: list[Item]

    @property
    def items(self) -> Iterator[Item]:
        for item in self.unique_items:
            for j in range(item.multiplicity):
                yield item


class MultipleKnapsackModel:
    def __init__(self, instance: MultipleKnapsackInstance):
        self.instance = instance

    def build(self, memory_limit=None):
        scip = Model('multiple-knapsack')

        if memory_limit is not None:
            scip.setRealParam('limits/memory', memory_limit)

        scip.setMaximize()

        i: ItemIndex
        j: KnapsackIndex

        # x_{i, j} = 1 iff item i is put into knapsack j
        x: Mapping[tuple[ItemIndex, KnapsackIndex], Variable] = {
            (i, j): scip.addVar(f'x[{i},{j}]', vtype='B', obj=item.profit)
            for i, item in enumerate(self.instance.items)
            for j, _ in enumerate(self.instance.knapsacks)
        }

        # Every item can be put into at most one knapsack
        for i, _ in enumerate(self.instance.items):
            scip.addCons(quicksum(x[i, j] for j, _ in enumerate(self.instance.knapsacks)) <= 1)

        # Weight capacity for each knapsack
        for j, knapsack in enumerate(self.instance.knapsacks):
            scip.addCons(quicksum(item.weight * x[i, j] for i, item in enumerate(self.instance.items)) <= knapsack.capacity)

        self.scip = scip
        self.x = x

    def add_item_orbitopes(self):
        i: ItemIndex
        j: KnapsackIndex

        for item in self.instance.unique_items:
            if item.multiplicity == 1:
                continue

            variables = [
                [self.x[i, j] for i, _ in [(k, it) for k, it in enumerate(self.instance.items) if it == item]]
                for j, _ in enumerate(self.instance.knapsacks)
            ]

            self.scip.addConsOrbitope(
                variables,
                OrbitopeType.FULL.value,
                usedynamicprop=False,
                resolveprop=False,
                ismodelcons=False,
                mayinteract=False,
                enforce=False,
            )

    def add_makespan_suborbitope_constraints(self, activation_handler: bool = True):
        i: ItemIndex
        j: KnapsackIndex

        variables = [
            [self.x[i, j] for j, _ in enumerate(self.instance.knapsacks)]
            for i, _ in enumerate(self.instance.items)
        ]

        cons = self.scip.addConsOrbitope(
            variables,
            OrbitopeType.PACKING.value,
            usedynamicprop=False,
            resolveprop=False,
            ismodelcons=False,
            mayinteract=False,
            enforce=False,
        )

        if activation_handler:
            self.scip.includeActivationMakespan()
            self.scip.registerConsActivationMakespan(cons, variables, [item.weight for item in self.instance.items])

    def add_subsym_ineqs(self, consecutive_pairs: bool = True):
        def iter_knapsacks():
            if consecutive_pairs:
                return pairwise(j for j, _ in enumerate(self.instance.knapsacks))
            else:
                return itertools.combinations((j for j, _ in enumerate(self.instance.knapsacks)), 2)

        W = 0
        i: ItemIndex
        j1: KnapsackIndex
        j2: KnapsackIndex
        for i, item_i in enumerate(self.instance.items):
            for j1, j2 in iter_knapsacks():
                alpha_plus = self.scip.addVar(f'alpha+[{i},{j1},{j2}]', lb=0, vtype='C')
                alpha_min = self.scip.addVar(f'alpha-[{i},{j1},{j2}]', lb=0, vtype='C')
                z_plus = self.scip.addVar(f'z+[{i},{j1},{j2}]', vtype='B')
                z_min = self.scip.addVar(f'z-[{i},{j1},{j2}]', vtype='B')

                z = z_plus + z_min
                alpha = quicksum(
                    item.weight * (self.x[k, j1] - self.x[k, j2])
                    for k, item in itertools.takewhile(lambda kitem: kitem[0] < i, enumerate(self.instance.items))
                )

                self.scip.addCons(alpha_plus <= W * z_plus)
                self.scip.addCons(alpha_min <= W * z_min)
                self.scip.addCons(alpha_plus + alpha_min >= z)
                self.scip.addCons(alpha == alpha_plus - alpha_min)
                self.scip.addCons(z_plus + z_min <= 1)

                self.scip.addCons(self.x[i, j2] <= z + self.x[i, j1])

            W += item_i.weight

    def add_subsym_ineqs_glob_orbitope(self, consecutive_pairs: bool = True):
        self.add_makespan_suborbitope_constraints(activation_handler=False)
        self.add_subsym_ineqs(consecutive_pairs)
