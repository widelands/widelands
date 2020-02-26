#!/usr/bin/python


"""This catches a closing brace followed by a character that is not within the.

[^] list. it catches code like "struct {T x}a;", which should be "struct.

{T x} a;". Characters after // are ignored.
"""

error_msg = 'Illegal character after closing brace.'

strip_macros = True
strip_comments_and_strings = True
regexp = r"""(?x)
\}[^ ,;})\"\\\n] # Match the } and finally the illegal characters
"""

forbidden = [
    r'struct {T x;}a;',
    r'"\\"}a;',
    r"\''}a;",

    # Eriks tests
    # illegal_character_after_closing_brace.cc
    '\tstruct {}A;',
    '\tstruct {}B;',
    '\tstruct {}C;',
    '\tstruct {}D;',
    '\tstruct {}E;',
    '\tstruct {}F;',
    '\tstruct {}G;',
    '\tstruct {}H;',
    '\tstruct {}I;',
    '\tstruct {}J;',
    '\tstruct {}K;',
    '\tstruct {}L;',
    '\tstruct {}M;',
    '\tstruct {}N;',
    '\tstruct {}O;',
    '\tstruct {}P;',
    '\tstruct {}Q;',
    '\tstruct {}R;',
    '\tstruct {}S;',
    '\tstruct {}T;',
    '\tstruct {}U;',
    '\tstruct {}V;',
    '\tstruct {}W;',
    '\tstruct {}X;',
    '\tstruct {}Y;',
    '\tstruct {}Z;',
    '\tstruct {}a;',
    '\tstruct {}b;',
    '\tstruct {}c;',
    '\tstruct {}d;',
    '\tstruct {}e;',
    '\tstruct {}f;',
    '\tstruct {}g;',
    '\tstruct {}h;',
    '\tstruct {}i;',
    '\tstruct {}j;',
    '\tstruct {}k;',
    '\tstruct {}l;',
    '\tstruct {}m;',
    '\tstruct {}n;',
    '\tstruct {}o;',
    '\tstruct {}p;',
    '\tstruct {}q;',
    '\tstruct {}r;',
    '\tstruct {}s;',
    '\tstruct {}t;',
    '\tstruct {}u;',
    '\tstruct {}v;',
    '\tstruct {}w;',
    '\tstruct {}x;',
    '\tstruct {}y;',
    '\tstruct {}z;',
    '\tstruct {}_;',
]

allowed = [
    r'}'+'\\',
    r'("expected [on failure {fail|repeat|ignore}] but found \"%s\""',
    '#macro blahblh }\\',
    r'  "   a:link    {text-decoration:none;}\n"',
    r'#define }hello',
    r'struct {T x;} a;',
    r'namespace A {struct B;};}',
    r'//}a',
    r'///}a',
    r'// {a|b}c',
    r"char a[][3] = {{'a', 'b', 'c'}, {'d', 'e', 'f'}, {'g', 'h', 'i'}}",
    r'"{a, b, c}"',
    r'"\"})"',
    r'"\"})" and even "\"})"',
    r'''("illegal state \"%s\" (must be one of {init, running, done)}",''',
    r'''("illegal state \"%s\" (must be one of {init, running, done})",''',  # Spot the difference ;)

    'int* blah() {return "anything";}\n',
]
