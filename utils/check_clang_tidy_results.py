#!/usr/bin/env python
# encoding: utf-8

import sys

def main():
    """Checks whether clang-tidy warnings that were previously cleaned have regressed."""
    if len(sys.argv) == 2:
        print('Checking clang-tidy results...')
    else:
        print(
            'Usage: check_clang_tidy_results.py <log_file>')
        return 1

    log_file = sys.argv[1]

    CLEAN_CHECKS = {
        '[google-readability-braces-around-statements]',
        '[hicpp-braces-around-statements]',
        '[readability-braces-around-statements]'
    }

    errors = 0

    with open(log_file) as checkme:
        contents = checkme.readlines()
        for line in contents:
            for check in CLEAN_CHECKS:
                if check in line:
                    print(line.strip())
                    errors = errors + 1

    if errors > 0:
        print('Found %s errors.' % errors)
        return 1
    else:
        print('Check has passed.')
        return 0


if __name__ == '__main__':
    sys.exit(main())
