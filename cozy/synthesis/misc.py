"""Miscellaneous procedures used during synthesis."""

from cozy.common import partition
from cozy.syntax import ETRUE, Exp, Query, TFunc, EVar, EAll, EImplies, EEq, ELambda, Stm, SNoOp, SDecl, SAssign, SSeq, SIf, SForEach, SCall
from cozy.target_syntax import TMap, EMakeMap2, EMapGet, SMapPut, SMapDel, SMapUpdate
from cozy.syntax_tools import fresh_var, free_vars, subst
from cozy.solver import solver_for_context
from cozy.contexts import RootCtx
from cozy.logging import task

def queries_equivalent(q1 : Query, q2 : Query, state_vars : [EVar], extern_funcs : { str : TFunc }, assumptions : Exp = ETRUE):
    """Determine whether two queries always return the same result.

    This function also checks that the two queries have identical preconditions.
    """

    with task("checking query equivalence", q1=q1.name, q2=q2.name):
        if q1.ret.type != q2.ret.type:
            return False
        q1args = dict(q1.args)
        q2args = dict(q2.args)
        if q1args != q2args:
            return False

        checker = solver_for_context(
            context=RootCtx(
                state_vars=state_vars,
                args=[EVar(a).with_type(t) for (a, t) in q1.args],
                funcs=extern_funcs),
            assumptions=assumptions)

        q1a = EAll(q1.assumptions)
        q2a = EAll(q2.assumptions)
        return checker.valid(EEq(q1a, q2a)) and checker.valid(EImplies(q1a, EEq(q1.ret, q2.ret)))

def pull_temps(s : Stm, decls_out : [SDecl], exp_is_bad) -> Stm:
    """Remove "bad" expressions from `s`.

    This procedure writes temporary definitions into `decls_out` for all
    expressions in `s` for which `exp_is_bad` returns True.  It returns `s`
    in terms of those temporary definitions.
    """

    def pull(e : Exp) -> Exp:
        if exp_is_bad(e):
            v = fresh_var(e.type)
            decls_out.append(SDecl(v, e))
            return v
        return e
    if isinstance(s, SNoOp):
        return s
    if isinstance(s, SSeq):
        s1 = pull_temps(s.s1, decls_out, exp_is_bad)
        s2 = pull_temps(s.s2, decls_out, exp_is_bad)
        return SSeq(s1, s2)
    if isinstance(s, SDecl):
        return SDecl(s.var, pull(s.val))
    if isinstance(s, SIf):
        cond = pull(s.cond)
        s1 = pull_temps(s.then_branch, decls_out, exp_is_bad)
        s2 = pull_temps(s.else_branch, decls_out, exp_is_bad)
        return SIf(cond, s1, s2)
    if isinstance(s, SForEach):
        bag = pull(s.iter)
        d_tmp = []
        body = pull_temps(s.body, d_tmp, exp_is_bad)
        to_fix, ok = partition(d_tmp, lambda d: s.loop_var in free_vars(d.val))
        decls_out.extend(ok)
        for d in to_fix:
            v = d.var
            mt = TMap(s.loop_var.type, v.type)
            m = EMakeMap2(bag, ELambda(s.loop_var, d.val)).with_type(mt)
            mv = fresh_var(m.type)
            md = SDecl(mv, m)
            decls_out.append(md)
            body = subst(body, { v.id : EMapGet(mv, s.loop_var).with_type(v.type) })
        return SForEach(s.loop_var, bag, body)
    if isinstance(s, SAssign):
        return SAssign(s.lhs, pull(s.rhs))
    if isinstance(s, SCall):
        return SCall(s.target, s.func, tuple(pull(arg) for arg in s.args))
    if isinstance(s, SMapDel):
        return SMapDel(s.map, pull(s.key))
    if isinstance(s, SMapPut):
        return SMapPut(s.map, pull(s.key), pull(s.value))
    if isinstance(s, SMapUpdate):
        key = pull(s.key)
        d_tmp = []
        change = pull_temps(s.change, d_tmp, exp_is_bad)
        for d in d_tmp:
            if s.val_var in free_vars(d.val):
                decls_out.append(SDecl(d.var, subst(d.val, { s.val_var.id : EMapGet(s.map, key).with_type(s.val_var.type) })))
            else:
                decls_out.append(d)
        return SMapUpdate(s.map, key, s.val_var, change)
    raise NotImplementedError(s)
