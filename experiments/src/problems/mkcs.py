from dataclasses import dataclass
from typing import Iterable, Dict, Tuple

from pyscipopt.symmetry import OrbitopeType

from graph.edgelist import GraphEdgeListNeighbors
from pyscipopt import Model, quicksum
from pyscipopt.scip import Variable


@dataclass(frozen=True)
class MkcsInstance:
    graph: GraphEdgeListNeighbors
    k: int


class MkcsModel:
    """Maximum k-colored subgraph problem"""

    def __init__(self, instance: MkcsInstance):
        self.graph = instance.graph
        self.k = instance.k

    @property
    def vertices(self) -> Iterable[int]:
        return self.graph.vertices

    @property
    def colors(self) -> Iterable[int]:
        return range(self.k)

    def build(self, memory_limit=None):
        """Build basic model."""
        scip = Model('mkcs')
        scip.setMaximize()

        if memory_limit is not None:
            scip.setRealParam('limits/memory', memory_limit)

        # x[i, k] indicates whether vertex i is colored with color k.
        # We assume that every vertex has equal weight in the objective function
        x: Dict[Tuple[int, int], Variable] = {(i, k): scip.addVar(f'x[i={i},k={k}]', vtype='B', obj=1)
                                              for i in self.vertices
                                              for k in self.colors}

        # Constraints for every edge (and color) to ensure edge endpoints are colored differently.
        for (i, j) in self.graph.edge_list:
            for k in self.colors:
                scip.addCons(
                    x[i, k] + x[j, k] <= 1,
                    name=f'valid_edge_endpoints_coloring[e=({i},{j}),k={k}]'
                )

        # Constraints for every vertex to ensure every vertex is colored at most once
        for i in self.vertices:
            scip.addCons(
                quicksum(x[i, k] for k in self.colors) <= 1,
                name=f'vertex_is_colored[i={i}]'
            )

        self.scip = scip
        self.x = x

    def add_color_orbitope_constraint(self, components_activation_handler: bool = False, all_color_pairs: bool = True, strategy: int = 0):
        matrix = [
            [self.x[i, k] for k in self.colors]
            for i in self.vertices
        ]

        cons = self.scip.addConsOrbitope(
            vars=matrix,
            orbitopetype=OrbitopeType.PACKING.value,
            usedynamicprop=not components_activation_handler,
            resolveprop=False,
            ismodelcons=False,
            mayinteract=False,
            enforce=False,
        )

        if components_activation_handler:
            # Build adjacency list
            adjacencies = [
                [j for j in self.graph.neighbors[i]]
                for i in self.graph.vertices
            ]

            self.scip.includeActivationColorComp()
            self.scip.registerConsActivationColorComp(cons, matrix, adjacencies, allcolorpairs=all_color_pairs, strategy=strategy)
