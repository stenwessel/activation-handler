##@file expr.pxi
#@brief In this file we implemenet the handling of expressions
#@details @anchor ExprDetails <pre> We have two types of expressions: Expr and GenExpr.
# The Expr can only handle polynomial expressions.
# In addition, one can recover easily information from them.
# A polynomial is a dictionary between `terms` and coefficients.
# A `term` is a tuple of variables
# For examples, 2*x*x*y*z - 1.3 x*y*y + 1 is stored as a
# {Term(x,x,y,z) : 2, Term(x,y,y) : -1.3, Term() : 1}
# Addition of common terms and expansion of exponents occur automatically.
# Given the way `Expr`s are stored, it is easy to access the terms: e.g.
# expr = 2*x*x*y*z - 1.3 x*y*y + 1
# expr[Term(x,x,y,z)] returns 1.3
# expr[Term(x)] returns 0.0
#
# On the other hand, when dealing with expressions more general than polynomials,
# that is, absolute values, exp, log, sqrt or any general exponent, we use GenExpr.
# GenExpr stores expression trees in a rudimentary way.
# Basically, it stores the operator and the list of children.
# We have different types of general expressions that in addition
# to the operation and list of children stores
# SumExpr: coefficients and constant
# ProdExpr: constant
# Constant: constant
# VarExpr: variable
# PowExpr: exponent
# UnaryExpr: nothing
# We do not provide any way of accessing the internal information of the expression tree,
# nor we simplify common terms or do any other type of simplification.
# The `GenExpr` is pass as is to SCIP and SCIP will do what it see fits during presolving.
#
# TODO: All this is very complicated, so we might wanna unify Expr and GenExpr.
# Maybe when consexpr is released it makes sense to revisit this.
# TODO: We have to think about the operations that we define: __isub__, __add__, etc
# and when to copy expressions and when to not copy them.
# For example: when creating a ExprCons from an Expr expr, we store the expression expr
# and then we normalize. When doing the normalization, we do
# ```
# c = self.expr[CONST]
# self.expr -= c
# ```
# which should, in princple, modify the expr. However, since we do not implement __isub__, __sub__
# gets called (I guess) and so a copy is returned.
# Modifying the expression directly would be a bug, given that the expression might be re-used by the user. </pre>


def _is_number(e):
    try:
        f = float(e)
        return True
    except ValueError: # for malformed strings
        return False
    except TypeError: # for other types (Variable, Expr)
        return False


def _expr_richcmp(self, other, op):
    if op == 1: # <=
        if isinstance(other, Expr):
            return (self - other) <= 0.0
        elif _is_number(other):
            return ExprCons(self, rhs=float(other))
        else:
            raise NotImplementedError
    elif op == 5: # >=
        if isinstance(other, Expr):
            return (self - other) >= 0.0
        elif _is_number(other):
            return ExprCons(self, lhs=float(other))
        else:
            raise NotImplementedError
    elif op == 2: # ==
        if isinstance(other, Expr):
            return (self - other) == 0.0
        elif _is_number(other):
            return ExprCons(self, lhs=float(other), rhs=float(other))
        else:
            raise NotImplementedError
    else:
        raise NotImplementedError


class Term:
    '''This is a monomial term'''

    __slots__ = ('vartuple', 'ptrtuple', 'hashval')

    def __init__(self, *vartuple):
        self.vartuple = tuple(sorted(vartuple, key=lambda v: v.ptr()))
        self.ptrtuple = tuple(v.ptr() for v in self.vartuple)
        self.hashval = sum(self.ptrtuple)

    def __getitem__(self, idx):
        return self.vartuple[idx]

    def __hash__(self):
        return self.hashval

    def __eq__(self, other):
        return self.ptrtuple == other.ptrtuple

    def __len__(self):
        return len(self.vartuple)

    def __add__(self, other):
        both = self.vartuple + other.vartuple
        return Term(*both)

    def __repr__(self):
        return 'Term(%s)' % ', '.join([str(v) for v in self.vartuple])


CONST = Term()

##@details Polynomial expressions of variables with operator overloading. \n
#See also the @ref ExprDetails "description" in the expr.pxi. 
cdef class Expr:
    
    def __init__(self, terms=None):
        '''terms is a dict of variables to coefficients.

        CONST is used as key for the constant term.'''
        self.terms = {} if terms is None else terms

        if len(self.terms) == 0:
            self.terms[CONST] = 0.0

    def __getitem__(self, key):
        if not isinstance(key, Term):
            key = Term(key)
        return self.terms.get(key, 0.0)

    def __iter__(self):
        return iter(self.terms)

    def __next__(self):
        try: return next(self.terms)
        except: raise StopIteration

    def __add__(self, other):
        left = self
        right = other

        if _is_number(self):
            assert isinstance(other, Expr)
            left,right = right,left
        terms = left.terms.copy()

        if isinstance(right, Expr):
            # merge the terms by component-wise addition
            for v,c in right.terms.items():
                terms[v] = terms.get(v, 0.0) + c
        elif _is_number(right):
            c = float(right)
            terms[CONST] = terms.get(CONST, 0.0) + c
        else:
            raise NotImplementedError
        return Expr(terms)

    def __iadd__(self, other):
        if isinstance(other, Expr):
            for v,c in other.terms.items():
                self.terms[v] = self.terms.get(v, 0.0) + c
        elif _is_number(other):
            c = float(other)
            self.terms[CONST] = self.terms.get(CONST, 0.0) + c
        else:
            raise NotImplementedError
        return self

    def __mul__(self, other):
        if _is_number(other):
            f = float(other)
            return Expr({v:f*c for v,c in self.terms.items()})
        elif _is_number(self):
            f = float(self)
            return Expr({v:f*c for v,c in other.terms.items()})
        elif isinstance(other, Expr):
            terms = {}
            for v1, c1 in self.terms.items():
                for v2, c2 in other.terms.items():
                    v = v1 + v2
                    terms[v] = terms.get(v, 0.0) + c1 * c2
            return Expr(terms)
        else:
            raise NotImplementedError

    def __truediv__(self,other):
        if _is_number(other):
            f = 1.0/float(other)
            return f * self
        raise NotImplementedError

    def __rtruediv__(self, other):
        ''' other / self '''
        if _is_number(self):
            f = 1.0/float(self)
            return f * other
        raise NotImplementedError

    def __pow__(self, other, modulo):
        if float(other).is_integer() and other >= 0:
            exp = int(other)
        else: # need to transform to GenExpr
            raise NotImplementedError

        res = 1
        for _ in range(exp):
            res *= self
        return res

    def __neg__(self):
        return Expr({v:-c for v,c in self.terms.items()})

    def __sub__(self, other):
        return self + (-other)

    def __radd__(self, other):
        return self.__add__(other)

    def __rmul__(self, other):
        return self.__mul__(other)

    def __rsub__(self, other):
        return -1.0 * self + other

    def __richcmp__(self, other, op):
        '''turn it into a constraint'''
        return _expr_richcmp(self, other, op)

    def normalize(self):
        '''remove terms with coefficient of 0'''
        self.terms =  {t:c for (t,c) in self.terms.items() if c != 0.0}

    def __repr__(self):
        return 'Expr(%s)' % repr(self.terms)

    def degree(self):
        '''computes highest degree of terms'''
        if len(self.terms) == 0:
            return 0
        else:
            return max(len(v) for v in self.terms)


cdef class ExprCons:
    '''Constraints with a polynomial expressions and lower/upper bounds.'''
    cdef public expr
    cdef public _lhs
    cdef public _rhs

    def __init__(self, expr, lhs=None, rhs=None):
        self.expr = expr
        self._lhs = lhs
        self._rhs = rhs
        assert not (lhs is None and rhs is None)
        self.normalize()

    def normalize(self):
        '''move constant terms in expression to bounds'''
        if isinstance(self.expr, Expr):
            c = self.expr[CONST]
            self.expr -= c
            assert self.expr[CONST] == 0.0
            self.expr.normalize()
        else:
            raise NotImplementedError

        if not self._lhs is None:
            self._lhs -= c
        if not self._rhs is None:
            self._rhs -= c


    def __richcmp__(self, other, op):
        '''turn it into a constraint'''
        if op == 1: # <=
           if not self._rhs is None:
               raise TypeError('ExprCons already has upper bound')
           assert self._rhs is None
           assert not self._lhs is None

           if not _is_number(other):
               raise TypeError('Ranged ExprCons is not well defined!')

           return ExprCons(self.expr, lhs=self._lhs, rhs=float(other))
        elif op == 5: # >=
           if not self._lhs is None:
               raise TypeError('ExprCons already has lower bound')
           assert self._lhs is None
           assert not self._rhs is None

           if not _is_number(other):
               raise TypeError('Ranged ExprCons is not well defined!')

           return ExprCons(self.expr, lhs=float(other), rhs=self._rhs)
        else:
            raise TypeError

    def __repr__(self):
        return 'ExprCons(%s, %s, %s)' % (self.expr, self._lhs, self._rhs)

    def __nonzero__(self):
        '''Make sure that equality of expressions is not asserted with =='''

        msg = """Can't evaluate constraints as booleans.

If you want to add a ranged constraint of the form
   lhs <= expression <= rhs
you have to use parenthesis to break the Python syntax for chained comparisons:
   lhs <= (expression <= rhs)
"""
        raise TypeError(msg)

def quicksum(termlist):
    '''add linear expressions and constants much faster than Python's sum
    by avoiding intermediate data structures and adding terms inplace
    '''
    result = Expr()
    for term in termlist:
        result += term
    return result

def quickprod(termlist):
    '''multiply linear expressions and constants by avoiding intermediate 
    data structures and multiplying terms inplace
    '''
    result = Expr() + 1
    for term in termlist:
        result *= term
    return result
