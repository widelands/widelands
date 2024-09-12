#!/usr/bin/python3

error_msg = 'Do not define type conversion operators. Use a regular function instead.'

regexp = r'\boperator\s*\w+\s*\('

allowed = [
    'bool operator ()(unsigned b);',
    '[[nodiscard]] int32_t value()',
]

forbidden = [
    'explicit operator bool();',
    '[[nodiscard]] operator size_t(void)',
]
