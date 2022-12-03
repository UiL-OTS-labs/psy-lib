#!/usr/bin/env python3
import unittest
import gi
import logging
import sys
from sys import getrefcount
gi.require_version('Psy', '0.1')
from gi.repository import Psy  # noqa: E402


class TestRefCount(unittest.TestCase):

    def setUp(self):
        '''Create a temporary window'''
        self.win = Psy.GtkWindow()

    def tearDown(self):
        del self.win

    def test_ref_count_starts_at_one(self):
        color = Psy.Color()
        circle = Psy.Circle(window=self.win)

        self.assertEqual(getrefcount(color), 2)
        self.assertEqual(getrefcount(circle), 2)

        self.assertEqual(color.ref_count, 1)
        self.assertEqual(circle.ref_count, 1)

    def test_assignment_increments_python_reference(self):
        c1 = Psy.Color()
        c2 = c1

        self.assertEqual(getrefcount(c2), 3)
        self.assertEqual(c1.ref_count, 1)

    def test_object_property(self):
        color = Psy.Color()  # one ref to a color
        circle = Psy.Circle(window=self.win)
        circle2 = Psy.Circle(window=self.win)

        circle.props.color = color
        # The color is now owned by the circle
        self.assertEqual(circle.props.color.ref_count, 2)

        circle2.props.color = color
        self.assertEqual(circle.props.color.ref_count, 3)
        self.assertEqual(circle.props.color, circle2.props.color)

        circle2.set_color(Psy.Color())
        self.assertEqual(color.ref_count, 2)

        circle.set_color(Psy.Color())
        self.assertEqual(color.ref_count, 1)

    def test_object_setter(self):
        color = Psy.Color()
        circle = Psy.Circle(window=self.win)

        circle.set_color(color)
        self.assertEqual(color.ref_count, 2)
        self.assertEqual(circle.props.color.ref_count, 2)

    def test_set_object_with_temp_objecr(self):
        circle = Psy.Circle(window=self.win)
        #circle.props.color = Psy.Color()
        circle.set_color(Psy.Color())

        # Only the circle has a reference
        self.assertEqual(circle.props.color.ref_count, 1)

    def test_set_object_in_constructor(self):
        color = Psy.Color()
        circle = Psy.Circle(window=self.win, color=color)
        self.assertEqual(circle.props.color.ref_count, 2)

        circle = Psy.Circle(window=self.win, color=Psy.Color())
        self.assertEqual(circle.props.color.ref_count, 1)


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger().setLevel(logging.DEBUG)
    unittest.main()
