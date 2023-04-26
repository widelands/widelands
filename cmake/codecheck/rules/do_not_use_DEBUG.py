#!/usr/bin/python

"""Prefer DEBUG."""

error_msg = 'Do not use #ifdef DEBUG, use #ifndef NDEBUG because this is what disable assert()s as well.'

regexp = r"""^#ifdef\s+DEBUG\b"""

forbidden = [
    '#ifdef DEBUG //  only in debug builds',
]

allowed = [
    '#ifndef NDEBUG'
]
