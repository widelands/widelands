#!/usr/bin/python


error_msg = 'Do not use std::rand(), use RNG::static_rand() from base/random.h.'

regexp = r"""std::s?rand()"""

forbidden = [
    """uint32_t i = std::rand();""",
    """std::srand(clock());""",
]

allowed = [
    """uint32_t i = RNG::static_rand();"""
]
