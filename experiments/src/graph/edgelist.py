from typing import List, Tuple, Dict, Set, Iterable, Optional


def _outside_neighbor(edge: Tuple[int, int], vertices: Set[int]) -> Optional[int]:
    i, j = edge
    if i in vertices and j not in vertices:
        return j
    if j in vertices and i not in vertices:
        return i
    return None


class GraphEdgeListNeighbors:
    """Data structure that stores a graph as an edge list and neighbor sets for single vertices."""
    def __init__(self, num_vertices: int, edge_list: List[Tuple[int, int]], neighbors: Dict[int, Set[int]]):
        self.num_vertices = num_vertices
        self.edge_list = edge_list
        self.neighbors = neighbors

    @staticmethod
    def read_from_col_file(path_to_file: str) -> 'GraphEdgeListNeighbors':
        num_vertices = 0
        edge_list = []
        neighbors = dict()

        with open(path_to_file, 'r') as f:
            for line in f:
                if line[0] == 'p':
                    # Read the number of vertices
                    num_vertices = int(line.split()[2])
                    neighbors = dict((i, set()) for i in range(num_vertices))

                elif line[0] == 'e':
                    split_line = line.split()
                    i = int(split_line[1]) - 1
                    j = int(split_line[2]) - 1
                    edge_list.append((i, j))
                    neighbors[i].add(j)
                    neighbors[j].add(i)

        return GraphEdgeListNeighbors(num_vertices, edge_list, neighbors)

    @staticmethod
    def from_edge_list(num_vertices: int, edge_list: List[Tuple[int, int]]) -> 'GraphEdgeListNeighbors':
        neighbors = dict((i, set()) for i in range(num_vertices))
        for (i, j) in edge_list:
            neighbors[i].add(j)
            neighbors[j].add(i)

        return GraphEdgeListNeighbors(num_vertices, edge_list, neighbors)

    @property
    def vertices(self) -> Iterable[int]:
        return range(self.num_vertices)

    def neighbors_set(self, vertices: Set[int]) -> Set[int]:
        return set().union(*(self.neighbors[v] for v in vertices)) - vertices
