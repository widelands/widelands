#!/usr/bin/python


"""
This catches %z in format strings which is not supported on mingw and is not
very compatible in either case.
"""

error_msg = "Do not use %z, consider using %\"PRIuS\" from log.h."

regexp = r"""%\S*z\S*\b"""

forbidden = [
    """("Fleet: %u, destination: %u, lastdock: %u, carrying: %zu\n",""",
]

allowed = [
    """("Fleet: %u, destination: %u, lastdock: %u, carrying: %"PRIuS"\n","""
]
