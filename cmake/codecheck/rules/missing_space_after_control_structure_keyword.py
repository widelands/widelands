#!/usr/bin/python


"""This catches code like "for(", which should be "for (".

It includes all keywords that denote control structures. But it should
not catch declarations, definitions or calls of functions whose name
happens to end with a control structure keyword, for example
"meanwhile(parameter)".
"""

error_msg = 'A space is mandatory after catch/for/if/switch/throw/while.'

regexp = r"""[^_a-zA-Z0-9](catch|for|if|switch|throw|while)\("""

forbidden = [
    '	for(;;)',
    '	catch(...) {',
    '	while(true)',
    "	throw('\n')",
]

allowed = [
    '	meanwhile(true)',
    '	while (true)',
    '	for (;;)',
    '	catch (...) {',
]
