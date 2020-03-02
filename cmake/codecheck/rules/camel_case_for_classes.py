#!/usr/bin/python

import re

strip_comments_and_strings = True


def evaluate_matches(lines, fn):
    errors = []

    for lineno, line in enumerate(lines):

        # Exception for Microsoft fix in src/network/network_system.h
        if line.count('in_port_t = uint16_t') or line.count('in_addr_t = uint32_t'):
            continue

        words = line.strip().split()
        if (len(words) < 2 or words[0] not in ['class', 'struct', 'enum', 'using']
                or words[1] == '{'):
            continue
        if words[0] == 'using' and words[1] == 'namespace' or words[1].startswith('std::'):
            continue
        if words[0] == 'enum':
            if words[1] == 'class':
                words.pop(1)
                if len(words) < 2:
                    continue
        if words[0] in ['class', 'struct', 'enum'] and words[-1] != '{':
            continue
        if re.match(r'([A-Z][a-z0-9]*)+', words[1]):
            continue
        errors.append(
            (fn, lineno + 1,
                "Use CapitalLetterWithCamelCase when naming an enum class, class, struct, or \"using\"."))

    return errors
# /end evaluate_matches


forbidden = [
    'class my_class {',
    'class myClass {',
    'struct my_struct {',
    'struct myStruct {',
    'using my_typedef {',
    'using myTypedef {',
    'enum class my_enum {',
    'enum class myEnum {'
    'enum my_enum {',
    'enum myEnum {'
]

allowed = [
    'class MyClass {',
    'struct MyStruct {',
    'using MyTypedef {',
    'enum class MyEnum {',
    'enum MyEnum {'
]
