#!/usr/bin/env python

import numpy as np
import os
import sys
import unittest

sys.path.append(os.path.normpath(os.path.dirname(os.path.abspath(__file__)) + '/..'))
import make_spritemap

class TestMinimumAverageCostRectangle(unittest.TestCase):
    def test_case1(self):
        FRAGMENT_COST = 4
        bitmask = np.array([
            [False, False, True],
            [False, False, False],
            [True, False, False],
            [True, False, False]
        ])
        cost, rectangle = make_spritemap.minimum_average_cost_rectangle(bitmask, FRAGMENT_COST=FRAGMENT_COST)
        self.assertEqual(rectangle, (2, 0, 4, 1))
        self.assertAlmostEqual(cost, (FRAGMENT_COST + 2) / 2.0)

class TestMinimumAverageCostGrow(unittest.TestCase):
    def test_basic_right(self):
        bitmask = np.array([
            [False, False, False, False],
            [False, False, False, False],
            [False, True,  True,  False],
            [False, True,  False, False],
            [False, False, False, False],
        ])
        cost, rectangle = make_spritemap.minimum_average_cost_grow(bitmask, (2, 1, 4, 2))
        self.assertEqual(rectangle, (2, 1, 4, 3))
        self.assertAlmostEqual(cost, 2.0)

        cost, rectangle = make_spritemap.minimum_average_cost_grow(bitmask, (2, 1, 4, 3))
        self.assertEqual(rectangle, (2, 1, 4, 3))
        self.assertEqual(cost, None)

    def test_basic_left(self):
        bitmask = np.array([
            [False, False, False, False],
            [False, False, False, False],
            [False, True,  False, False],
            [True,  False, False, False],
            [False, False, False, False],
        ])
        cost, rectangle = make_spritemap.minimum_average_cost_grow(bitmask, (2, 3, 4, 4))
        self.assertEqual(rectangle, (2, 0, 4, 4))
        self.assertAlmostEqual(cost, 3.0)

    def test_basic_vertical(self):
        bitmask = np.array([
            [False, True,  False, False],
            [False, False, False, False],
            [False, True,  False, False],
            [False, False, False, True],
            [False, False, False, False],
        ])
        cost, rectangle = make_spritemap.minimum_average_cost_grow(bitmask, (3, 1, 4, 2))
        self.assertEqual(rectangle, (2, 1, 4, 2))
        self.assertAlmostEqual(cost, 1.0)

        bitmask = np.array([
            [False, True,  False, False],
            [False, False, False, False],
            [False, False, False, False],
            [False, True,  False, False],
            [False, False, False, False],
        ])
        cost, rectangle = make_spritemap.minimum_average_cost_grow(bitmask, (2, 1, 3, 2))
        self.assertEqual(rectangle, (2, 1, 4, 2))
        self.assertAlmostEqual(cost, 1.0)


class TestComputeRectangleCovering(unittest.TestCase):
    def test_basic(self):
        """
        Tests a case where the optimal solution consists of a single
        minimum average cost rectangle.
        """
        FRAGMENT_COST = 4
        bitmask = np.array([
            [False, False, False, False],
            [False, False, False, False],
            [False, True,  True,  False],
            [False, True,  False, False],
            [False, False, False, False],
        ])
        cost, rectangles = make_spritemap.compute_rectangle_covering(
            bitmask,
            FRAGMENT_COST=FRAGMENT_COST
        )
        self.assertEqual(cost, FRAGMENT_COST + 4)
        self.assertItemsEqual(rectangles, [(2, 1, 4, 3)])

    def test_grow(self):
        """
        Tests a case where the optimal solution can be found
        by growing the initial minimum average cost rectangle.
        """
        FRAGMENT_COST = 4
        bitmask = np.array([
            [False, False, False, False, False, False],
            [False, False, False, False, False, False],
            [False, True,  True,  False, False, False],
            [False, True,  False, False, False, False],
            [False, True,  False, False, False, False],
            [False, True,  False, False, False, False],
            [False, False, False, False, False, False],
        ])
        cost, rectangles = make_spritemap.compute_rectangle_covering(
            bitmask,
            FRAGMENT_COST=FRAGMENT_COST
        )
        self.assertEqual(cost, FRAGMENT_COST + 8)
        self.assertItemsEqual(rectangles, [(2, 1, 6, 3)])

    def test_nogrow(self):
        """
        Similar to test_grow, but tests a situation in which
        growing the initial rectangle is in fact not optimal.
        """
        FRAGMENT_COST = 4
        bitmask = np.array([
            [False, False, False, False, False, False],
            [False, False, False, False, False, False],
            [False, True,  True,  True,  False, False],
            [False, True,  False, False, False, False],
            [False, True,  False, False, False, False],
            [False, True,  False, False, False, False],
            [False, False, False, False, False, False],
        ])
        cost, rectangles = make_spritemap.compute_rectangle_covering(
            bitmask,
            FRAGMENT_COST=FRAGMENT_COST
        )
        self.assertEqual(cost, 2 * FRAGMENT_COST + 6)
        self.assertItemsEqual(rectangles, [(2, 1, 6, 2), (2, 2, 3, 4)])

    def test_diag_growth_bug(self):
        """
        Regression test for a silly bug.
        """
        FRAGMENT_COST = 4
        bitmask = np.array([
            [True,  False, False, False],
            [False, False, False, False],
            [False, False, False, False],
            [False, False, False, True],
        ])
        cost, rectangles = make_spritemap.compute_rectangle_covering(
            bitmask,
            FRAGMENT_COST=FRAGMENT_COST
        )
        self.assertEqual(cost, 2 * FRAGMENT_COST + 2)
        self.assertItemsEqual(rectangles, [(0, 0, 1, 1), (3, 3, 4, 4)])

if __name__ == '__main__':
    unittest.main()
