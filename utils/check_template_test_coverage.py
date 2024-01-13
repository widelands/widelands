#!/usr/bin/env python3
# encoding: utf-8

import configparser
from glob import glob
import os.path
import re
import sys

# This script checks that all starting conditions of all tribes and all
# win conditions are used in the regression test suite.

print('Checking starting and win conditions test coverage')

basedir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))

datadir = os.path.join(basedir, 'data')
if not os.path.isdir(datadir):
    print('ERROR: Data directory ' + datadir + ' not found.')
    sys.exit(1)

# Collect win conditions
wcdir = os.path.join(datadir, 'scripting', 'win_conditions')
wc_init = os.path.join(wcdir, 'init.lua')
if not os.path.isfile(wc_init):
    print('ERROR: Win conditions config file not found at ' + wc_init)
    sys.exit(1)

with open(wc_init, 'r', encoding = 'utf-8') as fh:
    wc_regexp = re.compile('("[a-z_]+\\.lua")')
    win_conditions = {match.group(1): False
                      for match in [wc_regexp.search(line)
                      for line in fh.read().splitlines()] if match}
if len(win_conditions) < 1:
    print('ERROR: No win conditions found.')
    sys.exit(1)

print()
print('Win conditions:')
for wc in win_conditions:
    print('   ' + wc)

# Collect start conditions for all tribes
tribesdir = os.path.join(datadir, 'tribes', 'initialization')
tribes = sorted([td for td in glob(os.path.join(tribesdir, '*'))
                if os.path.isdir(os.path.join(td, 'starting_conditions'))])
if len(tribes) < 1:
    print('ERROR: No tribe directories found.')
    sys.exit(1)

start_conditions = {'"' + os.path.basename(td) + '"':
                    {'"' + os.path.basename(sc) + '"': False
                     for sc in sorted(glob(os.path.join(td, 'starting_conditions', '*')))}
                    for td in tribes}

if len(start_conditions) < 1:
    print('ERROR: No start conditions found.')
    sys.exit(1)

print()
print('Start conditions:')
for tr in start_conditions:
    print('   ' + tr + ':')
    for sc in start_conditions[tr]:
        print('      ' + sc)

# Collect tests
testdir = os.path.join(basedir, 'test', 'templates')
tests = sorted(glob(os.path.join(testdir, '*.wgt')))
if len(tests) < 1:
    print('ERROR: No tests found in directory ' + testdir)
    sys.exit(1)

unknown_wc = []
unknown_tribe = []
unknown_sc = dict()
no_script = []

# Collect uses
for test in tests:
    if not os.path.isfile(test[:-3] + 'lua'):
        no_script.append(test)
    cfg = configparser.ConfigParser()
    cfg.read(test)

    # This one is a bare minimum template, leaves all to defaults, including tribes, and start and
    # win conditions.
    if test == os.path.join(testdir, 'test_alldefaults.wgt'):
        continue

    for s in cfg.sections():
        section = cfg[s]

        # Test's win condition
        if s == 'global':
            if not section['win_condition'] in win_conditions:
                unknown_wc.append(section['win_condition'])
            else:
                win_conditions[section['win_condition']] = True

        # Each player's start condition
        if s.startswith('player_') and section['closed'] != '"true"' and \
           section['tribe'] != '':
            if not section['tribe'] in start_conditions:
                unknown_tribe.append(section['tribe'])
            else:
                if not section['init'] in start_conditions[section['tribe']]:
                    if not section['tribe'] in unknown_sc:
                        unknown_sc[section['tribe']] = []
                    unknown_sc[section['tribe']].append(section['init'])
                else:
                    start_conditions[section['tribe']][section['init']] = True

# Check unused
unused_wc = [wc for wc in win_conditions if not win_conditions[wc]]
unused_sc = dict()
for tribe in start_conditions:
    unused = [sc for sc in start_conditions[tribe] if not start_conditions[tribe][sc]]
    if len(unused) > 0:
        unused_sc[tribe] = unused

# Report
if len(unknown_wc) == 0 and len(unknown_tribe) == 0 and len(unknown_sc) == 0 and \
   len(unused_wc) == 0 and len(unused_sc) == 0 and len(no_script) == 0:
    print()
    print('Done, all starting and win conditions are covered by tests.')
    sys.exit(0)

if len(unknown_tribe) > 0:
    print()
    print('ERROR: Unknown tribes found in the tests:')
    print(unknown_tribe)
if len(unknown_sc) > 0:
    print()
    print('ERROR: Unknown start conditions found in the tests:')
    print(unknown_sc)
if len(unknown_wc) > 0:
    print()
    print('ERROR: Unknown win conditions found in the tests:')
    print(unknown_wc)

if len(unused_sc) > 0:
    print()
    print('ERROR: Start conditions not covered by tests:')
    print(unused_sc)
if len(unused_wc) > 0:
    print()
    print('ERROR: Win conditions not covered by tests:')
    print(unused_wc)

if len(no_script) > 0:
    print()
    print('ERROR: No scripts provided for tests:')
    print(no_script)

sys.exit(1)
