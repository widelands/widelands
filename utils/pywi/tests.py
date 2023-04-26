#!/usr/bin/env python
import config
import animation
"""Tests for various pywi modules."""

import os
import StringIO
import unittest

source_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.normpath(source_dir + '/../..')


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
        self.assertFalse('none' in s)

    def test_itersections(self):
        c = config.read(StringIO.StringIO(self.sample_text()))
        self.assertEqual(
            [name for name, section in c.itersections()], ['section'])

    def test_modify(self):
        c = config.read(StringIO.StringIO(self.sample_text()))
        s = c.get_section('section')
        s.set('foo', 'other')
        s.set('tst', 'blah')
        self.assertEqual(s['foo'], 'other')
        self.assertEqual(s['tst'], 'blah')
        stringio = StringIO.StringIO()
        c.write(stringio)
        c = config.read(StringIO.StringIO(stringio.getvalue()))
        s = c.get_section('section')
        self.assertEqual(s['foo'], 'other')
        self.assertEqual(s['baz'], 'quux')
        self.assertEqual(s['tst'], 'blah')

    def test_remove_section(self):
        c = config.read(StringIO.StringIO(self.sample_text()))
        s = c.make_section('other')
        s.set('x', 'y')
        c.remove_section('other')
        self.assertEqual(
            [name for name, section in c.itersections()], ['section'])
        s = c.get_section('section')
        self.assertItemsEqual([(k, v) for k, v in s.iterentries()], [
                              ('foo', 'bar'), ('baz', 'quux')])

    def test_make_section(self):
        c = config.read(StringIO.StringIO(self.sample_text()))
        s = c.make_section('section')
        self.assertEqual([(k, v) for k, v in s.iterentries()], [])
        s.set('foo', 'written')
        s2 = c.make_section('new')
        s2.set('bar', 'baz')
        stringio = StringIO.StringIO()
        c.write(stringio)
        c = config.read(StringIO.StringIO(stringio.getvalue()))
        s = c.get_section('section')
        self.assertItemsEqual([(k, v) for k, v in s.iterentries()], [
                              ('foo', 'written')])
        s2 = c.get_section('new')
        self.assertItemsEqual([(k, v)
                               for k, v in s2.iterentries()], [('bar', 'baz')])

    def test_makers(self):
        c = config.File()
        s1 = c.make_section('sec1')
        s1.set('0', '0')
        s1.set('1', '1')
        s2 = c.make_section('sec2')
        s2.set('a', 'a')
        s2.set('b', 'b')
        self.assertItemsEqual([(k, v) for k, v in s1.iterentries()], [
                              ('0', '0'), ('1', '1')])
        self.assertItemsEqual([(k, v) for k, v in s2.iterentries()], [
                              ('a', 'a'), ('b', 'b')])
        stringio = StringIO.StringIO()
        c.write(stringio)
        c = config.read(StringIO.StringIO(stringio.getvalue()))
        s1 = c.get_section('sec1')
        s2 = c.get_section('sec2')
        self.assertItemsEqual([(k, v) for k, v in s1.iterentries()], [
                              ('0', '0'), ('1', '1')])
        self.assertItemsEqual([(k, v) for k, v in s2.iterentries()], [
                              ('a', 'a'), ('b', 'b')])


class TestAnimation(unittest.TestCase):
    def test_load_conf(self):
        # This needs to be updated when the game data changes
        tests = [
            {
                'directory': root_dir + '/data/tribes/barbarians/carrier',
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
