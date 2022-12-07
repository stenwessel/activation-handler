import dataclasses
import itertools
import re
from collections import OrderedDict
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from typing import NewType, Iterator

from tabulate import tabulate

from pyscipopt import Model, quicksum, OrbitopeType
from pyscipopt.scip import Variable

from util import irange, pairwise

Time = NewType('Time', int)
UnitIndex = NewType('UnitIndex', int)
Production = NewType('Production', float)
Cost = NewType('Cost', float)


@dataclass(frozen=True)
class ProductionUnit:
    min_production: Production
    max_production: Production
    min_uptime: Time
    min_downtime: Time
    fixed_uptime_cost: Cost
    startup_cost: Cost
    production_cost: Cost


@dataclass(frozen=True)
class MucpInstance:
    max_time: Time
    demand: Mapping[Time, Production]
    unique_production_units: OrderedDict[ProductionUnit, int]

    def write(self) -> str:
        result = ''
        result += f'{self.max_time}\n'

        demand = [str(self.demand[i]) for i in irange(1, self.max_time)]
        result += f'{" ".join(demand)}\n'

        for pu, d in self.unique_production_units.items():
            result += f'{d} {" ".join([str(i) for i in dataclasses.astuple(pu)])}\n'

        return result[:-1]  # Truncate last newline

    @staticmethod
    def read(text: str) -> 'MucpInstance':
        lines = text.splitlines()

        max_time = int(lines[0])
        demand = {i + 1: float(d) for i, d in enumerate(lines[1].split(' '))}

        units = OrderedDict()
        for i in range(2, len(lines)):
            # Parse everything as int, except the last entry (as float)
            entries = lines[i].split(' ')
            d = int(entries[0])
            prod_cost = float(entries[-1])
            entries = [int(e) for e in entries[1:-1]]
            unit = ProductionUnit(*entries, production_cost=prod_cost)

            units[unit] = d

        return MucpInstance(max_time, demand, units)

    @staticmethod
    def read_orig(text: str) -> 'MucpInstance':
        # Read data to a dictionary first
        data = dict()
        for line in text.splitlines():
            match = re.match(r'(.*)? = (.*)', line)
            key = match.group(1)

            value = match.group(2)[2:-2].split(' ') if match.group(2).startswith('[') else int(match.group(2).rstrip(' ;'))

            data[key] = value

        # Gather production unit groups
        units = OrderedDict()
        first = -1
        unit = None
        for i in range(data['n']):
            if data['First'][i] == '1':
                # We found a new group
                unit = ProductionUnit(int(data['Pmin'][i]), int(data['Pmax'][i]), int(data['L'][i]), int(data['l'][i]), float(data['cf'][i]), float(data['c0'][i]), float(data['cp'][i]))
                first = i
            if data['Last'][i] == '1':
                # The group should now close
                units[unit] = i - first + 1

        # Gather demand
        demand = {i + 1: int(d) for i, d in enumerate(data['D'])}

        return MucpInstance(data['T'], demand, units)


class MucpBasicModel:
    """Basic IP model for min-up/min-down unit commitment problem."""

    def __init__(self, instance: MucpInstance):
        """Initializes the model.
        """
        if not all(j.min_uptime <= instance.max_time for j in instance.unique_production_units):
            raise ValueError('Minimum uptimes are not within time horizon.')

        if not all(j.min_downtime <= instance.max_time for j in instance.unique_production_units):
            raise ValueError('Minimum downtimes are not within time horizon.')

        self.max_time = instance.max_time
        self.unique_production_units = instance.unique_production_units
        self.demand = instance.demand

    @property
    def time_horizon(self) -> Sequence[Time]:
        return irange(1, self.max_time)

    @property
    def units(self) -> Sequence[tuple[UnitIndex, ProductionUnit]]:
        index = 1
        for unit, count in self.unique_production_units.items():
            for _ in range(count):
                yield index, unit
                index += 1

    def build(self, memory_limit=None):
        """Build basic model."""
        scip = Model('mucp')

        if memory_limit is not None:
            scip.setRealParam('limits/memory', memory_limit)

        # x[t, j] indicates whether unit j is up at time t
        x: Mapping[tuple[Time, UnitIndex], Variable] = {
            (t, j): scip.addVar(f'x[{t},{j}]', vtype='B', obj=unit.fixed_uptime_cost)
            for t in self.time_horizon
            for j, unit in self.units
        }

        # u[t, j] indicates whether unit j starts up at time t
        u: Mapping[tuple[Time, UnitIndex], Variable] = {
            (t, j): scip.addVar(f'u[{t},{j}]', vtype='B', obj=unit.startup_cost)
            for t in self.time_horizon
            for j, unit in self.units
        }

        # p[t, j] indicates the production amount of unit j at time t
        p: Mapping[tuple[Time, UnitIndex], Variable] = {
            (t, j): scip.addVar(f'p[{t},{j}]', vtype='C', obj=unit.production_cost)
            for t in self.time_horizon
            for j, unit in self.units
        }

        for j, unit in self.units:
            # Constraints to ensure minimum uptime
            for t in irange(unit.min_uptime, self.max_time):
                t = Time(t)
                scip.addCons(
                    quicksum(u[Time(t_), j] for t_ in irange(t - unit.min_uptime + 1, t)) <= x[t, j],
                    name=f'min_uptime[{j},{t}]'
                )

            # Constraints to ensure minimum downtime
            for t in irange(unit.min_downtime, self.max_time):
                t = Time(t)
                scip.addCons(
                    quicksum(u[Time(t_), j] for t_ in irange(t - unit.min_downtime + 1, t)) <= 1 - (0 if t == unit.min_downtime else x[Time(t - unit.min_downtime), j]),
                    name=f'min_downtime[{j},{t}]'
                )

            # Constraints to ensure start up is defined
            for t in irange(2, self.max_time):
                t = Time(t)
                scip.addCons(
                    u[t, j] >= x[t, j] - x[Time(t - 1), j],
                    name=f'startup[{j},{t}]'
                )

            # Constraints for production limits
            for t in self.time_horizon:
                t = Time(t)
                scip.addCons(
                    unit.min_production * x[t, j] <= p[t, j],
                    name=f'production_min[{j},{t}]'
                )
                scip.addCons(
                    p[t, j] <= unit.max_production * x[t, j],
                    name=f'production_max[{j},{t}]'
                )

        # Constraints to ensure demand is met
        for t in self.time_horizon:
            t = Time(t)
            scip.addCons(
                quicksum(p[t, j] for j, _ in self.units) >= self.demand[t],
                name=f'demand[{t}]'
            )

        self.scip = scip
        self.x = x
        self.u = u
        self.p = p

    def add_identical_units_orbitope_constraints(self, activation_handler: bool = False):
        x = self.x

        units = self.units

        if activation_handler:
            self.scip.includeActivationSuborbitope()

        for unit, count in self.unique_production_units.items():
            js = [j for j, _ in itertools.islice(units, count)]

            if count == 1:
                continue

            variables = [
                [x[t, j] for j in js]
                for t in self.time_horizon
            ]

            cons = self.scip.addConsOrbitope(
                variables,
                OrbitopeType.FULL.value,
                name=f'Orbitope[{unit}]',
                usedynamicprop=not activation_handler,
                resolveprop=False,
                ismodelcons=False,
                mayinteract=False,
                enforce=False,
            )

            if activation_handler:
                self.scip.registerConsActivationSuborbitope(cons, variables, unit.min_downtime, unit.min_uptime)

    def add_start_up_shut_down_ineqs(self, start_up: bool = True, shut_down: bool = True):
        rc = 0
        u, x = self.u, self.x
        for t in self.time_horizon:
            units = self.units

            for unit, count in self.unique_production_units.items():
                js = [j for j, _ in itertools.islice(units, count)]

                if count == 1:
                    continue

                for j1, j2 in pairwise(js):
                    if start_up and t >= unit.min_downtime + 1:
                        self.scip.addCons(
                            u[t, j2] <= x[Time(t - unit.min_downtime), j1]
                            + quicksum(u[t_, j1] for t_ in irange(t - unit.min_downtime + 1, t - 1))
                            + x[t, j1],
                            name=f'Start-up-ready[j={j1},j\'={j2},t={t},{unit}]'
                        )
                        rc += 1

                    if shut_down and t >= unit.min_uptime + 1:
                        self.scip.addCons(
                            x[Time(t - 1), j1] - x[t, j1] + u[t, j1] <= 1 - x[Time(t - unit.min_uptime), j2]
                            + quicksum(x[Time(t_ - 1), j2] - x[t_, j2] + u[t_, j2] for t_ in irange(t - unit.min_uptime + 1, t - 1))
                            + 1 - x[t, j2],
                            name=f'Shut-down-ready[j={j1},j\'={j2},t={t},{unit}]'
                        )
                        rc += 1

    def add_start_up_shut_down_orbisack_activation(self):
        rc = 0
        self.scip.includeActivationVarFix()

        x = self.x

        for t in self.time_horizon:
            units = self.units
            for unit, count in self.unique_production_units.items():
                js = [j for j, _ in itertools.islice(units, count)]

                if count == 1:
                    continue

                for j0, j1 in pairwise(js):
                    if t >= unit.min_downtime + 1:
                        cons = self.scip.addConsOrbisack(
                            [x[t_, j1] for t_ in irange(t, self.max_time)],
                            [x[t_, j0] for t_ in irange(t, self.max_time)],
                            ispporbisack=False,
                            isparttype=False,
                            ismodelcons=False,
                            initial=False,
                        )
                        self.scip.registerConsActivationVarFix(
                            cons,
                            [(x[t_, j], 0) for t_ in irange(t - unit.min_downtime, t - 1) for j in [j0, j1]]
                        )
                        rc += 1

                    if t >= unit.min_uptime + 1:
                        cons = self.scip.addConsOrbisack(
                            [x[t_, j1] for t_ in irange(t, self.max_time)],
                            [x[t_, j0] for t_ in irange(t, self.max_time)],
                            ispporbisack=False,
                            isparttype=False,
                            ismodelcons=False,
                            initial=False,
                        )
                        self.scip.registerConsActivationVarFix(
                            cons,
                            [(x[t_, j], 1) for t_ in irange(t - unit.min_uptime, t - 1) for j in [j0, j1]]
                        )
                        rc += 1

        print(f'Added {rc} subsym constraints')

    def print_solution(self):
        if (s := self.scip.getStatus()) != 'optimal':
            raise ValueError(f'Problem status is not "optimal" (status is {s})')

        sol = self.scip.getBestSol()
        p = self.p

        print(tabulate(
            [
                [None if self.scip.isZero(v := sol[p[t, j]]) else v for t in self.time_horizon]
                for j, _ in self.units
            ],
            headers=self.time_horizon,
            showindex=(j for j, _ in self.units),
        ))
