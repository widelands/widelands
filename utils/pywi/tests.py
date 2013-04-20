#!/usr/bin/env python
"""
Tests for various pywi modules
"""

import os
import StringIO
import unittest

source_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.normpath(source_dir + '/../..')

import animation
import config

class TestConfig(unittest.TestCase):
    def sample_text(self):
        return """
key = this is global

[section]
foo=bar # comment
# comment
baz= quux
"""

    def test_read(self):
        c = config.read(StringIO.StringIO(self.sample_text()))
        s = c.get_section('section')
        self.assertEqual(s['foo'], 'bar')
        self.assertEqual(s['baz'], 'quux')


class TestAnimation(unittest.TestCase):
    def test_load_conf(self):
        # This needs to be updated when the game data changes
        tests = [
            {
                'directory': root_dir + '/tribes/barbarians/carrier',
                'animation': 'idle',
                'shape': (28, 21),
                'hotspot': (21, 14),
                'has_player_color': True,
                'nrframes': 100,
            },
        ]

        for test in tests:
            anim = animation.load_conf(test['directory'], test['animation'])
            self.assertEqual(anim.shape, test['shape'])
            self.assertEqual(anim.has_player_color, test['has_player_color'])
            self.assertEqual(anim.get_nrframes(), test['nrframes'])


if __name__ == '__main__':
    unittest.main()
