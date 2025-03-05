#!/usr/bin/env python3
# encoding: utf-8

"""Checks whether clang-tidy warnings that were previously cleaned have
regressed."""

import re
import sys

# Checks list: https://clang.llvm.org/extra/clang-tidy/checks/list.html
SUPPRESSED_CHECKS = {
    # These are also disabled in the project .clang-tidy config file

    # TODO(Nordfriese): Investigate which android-* and fuchsia-* checks we want to enable
    # (currently they're all excluded in the clang-tidy invokation)

    # Checks we strictly need and cannot clean up for the time being
    'cert-dcl50-cpp',  # We need this for our logger
    'cppcoreguidelines-pro-type-vararg',  # We need this for our logger
    'hicpp-vararg',
    'llvm-header-guard',  # We have our own header style with a codecheck rule on it

    # Checks that probably do not make sense for us
    # (up for discussion; see https://github.com/widelands/widelands/discussions/5262)
    'altera-id-dependent-backward-branch',
    'altera-unroll-loops',
    'boost-use-to-string',
    'bugprone-reserved-identifier',
    'bugprone-unchecked-optional-access',
    'clang-analyzer-optin.core.EnumCastOutOfRange',
    'cppcoreguidelines-avoid-c-arrays',
    'cppcoreguidelines-avoid-const-or-ref-data-members',
    'cppcoreguidelines-avoid-do-while',
    'cppcoreguidelines-avoid-goto',
    'cppcoreguidelines-no-malloc',
    'cppcoreguidelines-non-private-member-variables-in-classes',
    'cppcoreguidelines-owning-memory',
    'cppcoreguidelines-pro-bounds-constant-array-index',
    'cppcoreguidelines-pro-bounds-pointer-arithmetic',
    'cppcoreguidelines-pro-type-const-cast',
    'cppcoreguidelines-pro-type-reinterpret-cast',
    'cppcoreguidelines-pro-type-union-access',
    'cppcoreguidelines-pro-bounds-array-to-pointer-decay',
    'google-default-arguments',
    'hicpp-avoid-c-arrays',
    'hicpp-avoid-goto',
    'hicpp-no-array-decay',
    'hicpp-no-malloc',
    'hicpp-uppercase-literal-suffix',
    'hicpp-use-auto',
    'llvmlibc-callee-namespace',
    'llvmlibc-implementation-in-namespace',
    'llvmlibc-inline-function-decl',
    'llvmlibc-restrict-system-libc-headers',
    'misc-no-recursion',
    'misc-non-private-member-variables-in-classes',
    'misc-use-anonymous-namespace',
    'modernize-raw-string-literal',
    'modernize-replace-disallow-copy-and-assign-macro',
    'modernize-return-braced-init-list',
    'modernize-avoid-c-arrays',
    'modernize-use-auto',
    'modernize-use-trailing-return-type',
    'performance-avoid-endl',
    'performance-no-int-to-ptr',
    'readability-avoid-return-with-void-value',
    'readability-avoid-unconditional-preprocessor-if',
    'readability-identifier-length',
    'readability-math-missing-parentheses',
    'readability-redundant-inline-specifier',
    'readability-uppercase-literal-suffix',
    'readability-use-std-min-max',

    # Checks we probably want to clean up sometime (discussible; see link above)
    'altera-struct-pack-align',
    'bugprone-easily-swappable-parameters',
    'bugprone-empty-catch',
    'bugprone-exception-escape',
    'bugprone-macro-parentheses',
    'bugprone-multi-level-implicit-pointer-conversion',
    'bugprone-narrowing-conversions',
    'bugprone-not-null-terminated-result',
    'bugprone-signed-char-misuse',
    'bugprone-too-small-loop-variable',
    'bugprone-unused-return-value',
    'cert-err33-c',
    'cert-err58-cpp',
    'cert-msc32-c',
    'cert-msc51-cpp',
    'clang-analyzer-core.CallAndMessage',
    'clang-analyzer-core.NonNullParamChecker',
    'clang-analyzer-cplusplus.NewDelete',
    'clang-analyzer-cplusplus.NewDeleteLeaks',
    'clang-analyzer-optin.cplusplus.UninitializedObject',
    'clang-analyzer-optin.cplusplus.VirtualCall',
    'clang-diagnostic-documentation-unknown-command',
    'concurrency-mt-unsafe',
    'cppcoreguidelines-avoid-magic-numbers',
    'cppcoreguidelines-avoid-non-const-global-variables',
    'cppcoreguidelines-init-variables',
    'cppcoreguidelines-macro-usage',
    'cppcoreguidelines-narrowing-conversions',
    'cppcoreguidelines-prefer-member-initializer',
    'cppcoreguidelines-pro-type-member-init',
    'cppcoreguidelines-slicing',
    'cppcoreguidelines-special-member-functions',
    'google-readability-casting',
    'google-readability-function-size',
    'google-runtime-references',
    'hicpp-function-size',
    'hicpp-member-init',
    'hicpp-multiway-paths-covered',
    'hicpp-named-parameter',
    'hicpp-signed-bitwise',
    'hicpp-special-member-functions',
    'misc-confusable-identifiers',
    'misc-const-correctness',
    'misc-include-cleaner',
    'misc-header-include-cycle',
    'misc-macro-parentheses',
    'misc-use-internal-linkage',
    'modernize-make-unique',
    'modernize-pass-by-value',
    'modernize-use-default-member-init',
    'performance-enum-size',
    'performance-unnecessary-value-param',
    'readability-avoid-nested-conditional-operator',
    'readability-const-return-type',
    'readability-convert-member-functions-to-static',
    'readability-enum-initial-value',
    'readability-function-cognitive-complexity',
    'readability-function-size',
    'readability-magic-numbers',
    'readability-named-parameter',
    'readability-redundant-casting',
    'readability-redundant-member-init',
    'readability-static-accessed-through-instance',
    'readability-suspicious-call-argument',
}

CHECK_REGEX = re.compile(r'.*\[([A-Za-z0-9.,-]+)\]$')


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

    errors = []

    with open(log_file) as checkme:
        contents = checkme.readlines()
        for line in contents:
            if line in errors:
                continue
            if 'src/third_party' in line or '/usr/include' in line:
                continue
            checks_match = CHECK_REGEX.match(line)
            if not checks_match:
                continue
            failed_checks = checks_match.group(1).split(',')
            for check in failed_checks:
                # We're not piloting alpha-level checks
                if 'clang-analyzer-alpha' in check:
                    continue
                if not check in SUPPRESSED_CHECKS:
                    print(line.strip())
                    errors.append(line)
                    break

    if len(errors) > 0:
        print('########################################################')
        print('########################################################')
        print('###                                                  ###')
        print('###   Found %5s error(s)                           ###'
              % len(errors))
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
