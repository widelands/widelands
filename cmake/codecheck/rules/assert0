#!/usr/bin/env python -tt
# encoding: utf-8
#

"""
Use a descriptive macro instead of assert(false);
"""

error_msg="Use NEVER_HERE() from base/macros.h here."

regexp = r"""assert *\( *(0|false) *\)"""

forbidden = [
    "assert(0)",
    "assert(false)",
]

allowed = [
    "NEVER_HERE()",
]
