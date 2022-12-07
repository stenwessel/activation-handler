import itertools
import math
import operator
from enum import Enum
from functools import reduce
from numbers import Number, Real
from statistics import geometric_mean
from typing import Collection

from dataclass_csv import DataclassReader


def pairwise(iterable):
    # pairwise('ABCDEFG') --> AB BC CD DE EF FG
    a, b = itertools.tee(iterable)
    next(b, None)
    return zip(a, b)


def irange(start, stop, step=1):
    """Range where `stop` is inclusive."""
    return range(start, stop + 1, step)


class NewTypeDataclassReader(DataclassReader):

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.type_hints = {name: t.__supertype__ if t.__class__.__qualname__ == 'NewType' else t for name, t in self.type_hints.items()}


def shifted_geometric_mean(data: Collection[Real], shift: Real) -> Real:
    return geometric_mean((i + shift for i in data)) - shift
