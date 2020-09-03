#!/usr/bin/env python
# encoding: utf-8

"""Checks whether clang-tidy warnings that were previously cleaned have
regressed."""

import re
import sys

# Checks list: https://clang.llvm.org/extra/clang-tidy/checks/list.html
SUPPRESSED_CHECKS = {
    '[android-cloexec-fopen]',
    '[boost-use-to-string]',
    '[bugprone-integer-division]',
    '[cert-dcl50-cpp]',  # We need this for our logger
    '[cert-err58-cpp]',
    '[clang-analyzer-core.NonNullParamChecker]',
    '[clang-analyzer-core.UndefinedBinaryOperatorResult]',
    '[clang-analyzer-cplusplus.NewDelete]',
    '[clang-analyzer-cplusplus.NewDeleteLeaks]',
    '[clang-analyzer-optin.cplusplus.VirtualCall]',
    '[cppcoreguidelines-no-malloc]',
    '[cppcoreguidelines-owning-memory]',
    '[cppcoreguidelines-pro-bounds-array-to-pointer-decay]',
    '[cppcoreguidelines-pro-bounds-constant-array-index]',
    '[cppcoreguidelines-pro-bounds-pointer-arithmetic]',
    '[cppcoreguidelines-pro-type-const-cast]',
    '[cppcoreguidelines-pro-type-member-init]',
    '[cppcoreguidelines-pro-type-reinterpret-cast]',
    '[cppcoreguidelines-pro-type-union-access]',
    '[cppcoreguidelines-pro-type-vararg]',  # We need this for our logger
    '[cppcoreguidelines-slicing]',
    '[cppcoreguidelines-special-member-functions]',
    '[fuchsia-default-arguments]',
    '[fuchsia-overloaded-operator]',
    '[google-default-arguments]',
    '[google-readability-function-size]',
    '[google-runtime-references]',
    '[hicpp-member-init]',
    '[hicpp-move-const-arg]',
    '[hicpp-no-array-decay]',
    '[hicpp-no-malloc]',
    '[hicpp-noexcept-move]',
    '[hicpp-signed-bitwise]',
    '[hicpp-special-member-functions]',
    '[hicpp-use-auto]',
    '[hicpp-use-emplace]',
    '[hicpp-use-equals-default]',
    '[hicpp-use-nullptr]',
    '[hicpp-use-override]',
    '[hicpp-vararg]',
    '[llvm-namespace-comment]',
    '[misc-macro-parentheses]',
    '[misc-misplaced-widening-cast]',
    '[misc-redundant-expression]',
    '[misc-string-integer-assignment]',
    '[misc-suspicious-enum-usage]',
    '[misc-suspicious-string-compare]',
    '[misc-uniqueptr-reset-release]',
    '[misc-unused-using-decls]',
    '[modernize-loop-convert]',
    '[modernize-make-shared]',
    '[modernize-make-unique]',
    '[modernize-pass-by-value]',
    '[modernize-raw-string-literal]',
    '[modernize-return-braced-init-list]',
    '[modernize-use-auto]',
    '[modernize-use-bool-literals]',
    '[modernize-use-default-member-init]',
    '[modernize-use-emplace]',
    '[modernize-use-equals-default]',
    '[modernize-use-nullptr]',
    '[modernize-use-override]',
    '[readability-container-size-empty]',
    '[readability-delete-null-pointer]',
    '[readability-else-after-return]',
    '[readability-implicit-bool-conversion]',
    '[readability-inconsistent-declaration-parameter-name]',
    '[readability-named-parameter]',
    '[readability-non-const-parameter]',
    '[readability-redundant-control-flow]',
    '[readability-redundant-member-init]',
    '[readability-redundant-smartptr-get]',
    '[readability-redundant-string-cstr]',
    '[readability-redundant-string-init]',
    '[readability-simplify-boolean-expr]',
    '[readability-static-accessed-through-instance]',
    '[readability-static-definition-in-anonymous-namespace]'
}

CHECK_REGEX = re.compile(r'.*\[([A-Za-z0-9.-]+)\]$')


def main():
    """Checks whether clang-tidy warnings that were previously cleaned have
    regressed."""
    if len(sys.argv) == 2:
        print('########################################################')
        print('########################################################')
        print('###                                                  ###')
        print('###   Checking clang-tidy results                    ###')
        print('###                                                  ###')
        print('########################################################')
        print('########################################################')
    else:
        print(
            'Usage: check_clang_tidy_results.py <log_file>')
        return 1

    log_file = sys.argv[1]

    errors = 0

    with open(log_file) as checkme:
        contents = checkme.readlines()
        for line in contents:
            if 'third_party' in line:
                continue
            # We're not piloting alpha-level checks
            if 'clang-analyzer-alpha' in line:
                continue
            check_suppressed = False
            for check in SUPPRESSED_CHECKS:
                if check in line:
                    check_suppressed = True
                    break
            if not check_suppressed and CHECK_REGEX.match(line):
                print(line.strip())
                errors = errors + 1

    if errors > 0:
        print('########################################################')
        print('########################################################')
        print('###                                                  ###')
        print('###   Found %s error(s)                               ###'
              % errors)
        print('###                                                  ###')
        print('########################################################')
        print('########################################################')
        print('Information about the checks can be found on:')
        print('https://clang.llvm.org/extra/clang-tidy/checks/list.html')
        return 1

    print('###                                                  ###')
    print('###   Check has passed                               ###')
    print('###                                                  ###')
    print('########################################################')
    print('########################################################')
    return 0


if __name__ == '__main__':
    sys.exit(main())
