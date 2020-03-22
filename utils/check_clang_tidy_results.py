#!/usr/bin/env python
# encoding: utf-8

import sys

def main():
    """Checks whether clang-tidy warnings that were previously cleaned have regressed."""
    if len(sys.argv) == 2:
        print('#######################################')
        print('#######################################')
        print('###                                 ###')
        print('###   Checking clang-tidy results   ###')
        print('###                                 ###')
        print('#######################################')
        print('#######################################')
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
            if not 'third_party' in line:
                for check in CLEAN_CHECKS:
                    if check in line:
                        print(line.strip())
                        errors = errors + 1

    if errors > 0:
        print('#######################################')
        print('#######################################')
        print('###                                 ###')
        print('###   Found %s error(s)             ###' % errors)
        print('###                                 ###')
        print('#######################################')
        print('#######################################')
        return 1
    else:
        print('#######################################')
        print('#######################################')
        print('###                                 ###')
        print('###   Check has passed              ###')
        print('###                                 ###')
        print('#######################################')
        print('#######################################')
        return 0


if __name__ == '__main__':
    sys.exit(main())
