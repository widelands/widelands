#!/usr/bin/env python -tt
# encoding: utf-8
#

"""Python CPPChecker broke down because of this wrong line.

I add a style fix for it now, so it won't bother us again.
"""

error_msg = 'Garbage tokens after include.'

regexp = r"""(?x)
^\#include\s*
((".*?")|(<.*?>))
[^\s]+$"""

forbidden = [
    r"""#include "io/filesystem/layered_filesystem.h"io/filesystem/""",
    r"""#include <file.h>io/filesystem/""",
]

allowed = [
    r'#include "file.h"',
    r"""#include <file.h>""",
    r"""#include <file.h>  \t""",  # Trailing whitespaces are not of interest for this rule
    r"""#include "file.h" \t """,
]
