"""This module is created in order to add support for arithmetic and comparison
operators. The only thing you have to do is import this module, and the module
will make the operators for Psy.TimePoint and Psy.Durtion, This module
tries to be agnostic to the version of psylib, hence it is recommended
to specify gi.require_version("Psy", "your-desired-version") prior to
importing this module.
This module works via adding __lt__, __add__ etc. On the class of Psy.TimePoint
and Psy.Module and perhaps more to come.
"""

from gi.repository import Psy
import typing as t

dur_or_tp = t.Union[Psy.TimePoint, Psy.Duration]
TP = Psy.TimePoint
DUR = Psy.Duration


def _timepoint_op_add(self: Psy.TimePoint, other: Psy.Duration) -> TP:
    """You can add an duration to a TimePoint"""
    #    assert isinstance(self, Psy.TimePoint)
    #    if not isinstance(other, Psy.Duration):
    #        raise TypeError(f"{type(self)} + {type(other)} is not supported")
    return self.add(other)


def _timepoint_op_subtract(self: Psy.TimePoint, other: dur_or_tp) -> dur_or_tp:
    """You can subtract a Duration and another TimePoint from a TimePoint
    in the first case, you'll get a new duration, in the latter case you'll
    get an other TimePoint with a duration offset in the past(or future when
    the duration is negative.
    """
    if isinstance(other, Psy.TimePoint):
        return self.subtract(other)
    else:
        return self.subtract_dur(other)


def _timepoint_op_le(self: TP, other: TP) -> bool:
    """Checks whether self < other"""
    assert isinstance(self, TP)
    return self.less_equal(other) if isinstance(other, TP) else NotImplemented


def _timepoint_op_lt(self: TP, other: TP) -> bool:
    """Checks whether self < other"""
    assert isinstance(self, TP)
    return self.less(other) if isinstance(other, TP) else NotImplemented


def _timepoint_op_eq(self: TP, other: TP) -> bool:
    """Checks whether two instances of Psy.TimePoint are identical"""
    assert isinstance(self, TP)
    return self.equal(other) if isinstance(other, TP) else NotImplemented


def _timepoint_op_ne(self: TP, other: TP) -> bool:
    """Checks whether two instances of Psy.TimePoint are identical"""
    assert isinstance(self, TP)
    return self.not_equal(other) if isinstance(other, TP) else NotImplemented


def _timepoint_op_gt(self: TP, other: TP) -> bool:
    """Checks whether self < other"""
    assert isinstance(self, TP)
    return self.greater(other) if isinstance(other, TP) else NotImplemented


def _timepoint_op_ge(self: TP, other: TP) -> bool:
    """Checks whether self < other"""
    assert isinstance(self, TP)
    return self.greater_equal(other) if isinstance(other, TP) else NotImplemented


Psy.TimePoint.__add__ = _timepoint_op_add
Psy.TimePoint.__sub__ = _timepoint_op_subtract

Psy.TimePoint.__lt__ = _timepoint_op_lt
Psy.TimePoint.__le__ = _timepoint_op_le
Psy.TimePoint.__eq__ = _timepoint_op_eq
Psy.TimePoint.__ne__ = _timepoint_op_ne
Psy.TimePoint.__gt__ = _timepoint_op_gt
Psy.TimePoint.__ge__ = _timepoint_op_ge

# Psy.Duration.__add__ = _duration_op_add
# Psy.Duration.__sub__ = _duration_op_sub
