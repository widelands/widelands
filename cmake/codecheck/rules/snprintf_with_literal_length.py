#!/usr/bin/python


"""This catches code like:

* char buffer[25];
* snprintf(buffer, 25, "format string");

It should of course be:

* char buffer[25];
* snprintf(buffer, sizeof(buffer), "format string");
"""

error_msg = 'Do not use literal length in snprintf. Use sizeof(buffer).'

regexp = r"""snprintf *\( *[_a-zA-Z][_a-zA-Z0-9]* *, *\d+"""

forbidden = [
    'snprintf(buffer, 25, "format string");',
]

allowed = [
    'snprintf(buffer, sizeof(buffer), "format string");',
]
