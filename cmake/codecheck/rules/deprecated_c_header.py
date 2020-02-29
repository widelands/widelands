#!/usr/bin/python


"""Checks for includes of deprecated c headers."""

error_msg = 'You have included a deprecated C header. Remove it!'

regexp = r"""^# *include +["<](?:assert|complex|type|errno|fenv|float|inttypes|iso646|limits|locale|math|setjmp|signal|stdarg|stdbool|stddef|stdio|stdlib|string|tgmath|time|wchar|wctype)\.h[">]"""

forbidden = [
    '#include <assert.h>',
    '#include <complex.h>',
    '#include <type.h>',
    '#include <errno.h>',
    '#include <fenv.h>',
    '#include <float.h>',
    '#include <inttypes.h>',
    '#include <iso646.h>',
    '#include <limits.h>',
    '#include <locale.h>',
    '#include <math.h>',
    '#include <setjmp.h>',
    '#include <signal.h>',
    '#include <stdarg.h>',
    '#include <stdbool.h>',
    '#include <stddef.h>',
    '#include <stdio.h>',
    '#include <stdlib.h>',
    '#include <string.h>',
    '#include <tgmath.h>',
    '#include <time.h>',
    '#include <wchar.h>',
    '#include <wctype.h>',
]

allowed = [
    '#include <cassert>',
    '#include <ccomplex>',
    '#include <ctype>',
    '#include <cerrno>',
    '#include <cfenv>',
    '#include <cfloat>',
    '#include <cinttypes>',
    '#include <ciso646>',
    '#include <climits>',
    '#include <clocale>',
    '#include <cmath>',
    '#include <csetjmp>',
    '#include <csignal>',
    '#include <cstdarg>',
    '#include <cstdbool>',
    '#include <cstddef>',
    '#include <cstdio>',
    '#include <cstdlib>',
    '#include <cstring>',
    '#include <ctgmath>',
    '#include <ctime>',
    '#include <cwchar>',
    '#include <cwctype>',
]
