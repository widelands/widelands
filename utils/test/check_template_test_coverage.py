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

basedir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir, os.path.pardir))

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
test_basedir = os.path.join(basedir, 'test')
testdir = os.path.join(test_basedir, 'templates')
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

    # Allow skipping templates that have below comment as first line.
    # Can be used for local tests as needed or desired.
    # There is one official test that uses this, because its purpose is testing
    # default values. Normally we don't allow that, because it introduces
    # uncertainty into the test environment.
    firstline = ''
    with open(test, mode='r', encoding='utf-8') as f:
        firstline = f.readline().strip()
    if firstline == '# coverage test: ignore':
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

# Check tribes missing from test/maps/all_tribes.wmf
missing_in_alltribes = []
allmapsdir = os.path.join(test_basedir, 'maps', 'all_tribes.wmf', 'scripting')
for tribe in start_conditions:
    tribe = tribe.strip('"')
    script = os.path.join(allmapsdir, f'test_{ tribe }.lua')
    if not os.path.isfile(script):
        missing_in_alltribes.append(tribe)

# Report
failed = False
def check_empty(err_list, message):
    global failed
    if len(err_list) > 0:
        failed = True
        print()
        print('ERROR:', message)
        print(err_list)

check_empty(unknown_tribe, 'Unknown tribes found in the tests:')
check_empty(unknown_sc, 'Unknown start conditions found in the tests:')
check_empty(unknown_wc, 'Unknown win conditions found in the tests:')

check_empty(unused_sc, 'Start conditions not covered by tests:')
check_empty(unused_wc, 'Win conditions not covered by tests:')

check_empty(no_script, 'No scripts provided for tests:')

check_empty(missing_in_alltribes, 'Tribes not covered by test/maps/all_tribes.wmf:')

if failed:
    sys.exit(1)

print()
print('Done, all starting and win conditions are covered by tests.')
sys.exit(0)
