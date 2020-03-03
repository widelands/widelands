#!/usr/bin/python


"""Catches illegal characters after semicolon."""

error_msg = 'Illegal character after semicolon'''

strip_macros = True
strip_comments_and_strings = True
regexp = r"""(?x)
;[\w+*&/-]
"""

forbidden = [
    # Eriks tests
    # illegal_character_after_semicolon.cc
    '\tfloat A;A;',
    '\tfloat a;a;',
    '\tfloat B;B;',
    '\tfloat b;b;',
    '\tfloat C;C;',
    '\tfloat c;c;',
    '\tfloat D;D;',
    '\tfloat d;d;',
    '\tfloat E;E;',
    '\tfloat e;e;',
    '\tfloat F;F;',
    '\tfloat f;f;',
    '\tfloat G;G;',
    '\tfloat g;g;',
    '\tfloat H;H;',
    '\tfloat h;h;',
    '\tfloat I;I;',
    '\tfloat i;i;',
    '\tfloat J;J;',
    '\tfloat j;j;',
    '\tfloat K;K;',
    '\tfloat k;k;',
    '\tfloat L;L;',
    '\tfloat l;l;',
    '\tfloat M;M;',
    '\tfloat m;m;',
    '\tfloat N;N;',
    '\tfloat n;n;',
    '\tfloat O;O;',
    '\tfloat o;o;',
    '\tfloat P;P;',
    '\tfloat p;p;',
    '\tfloat Q;Q;',
    '\tfloat q;q;',
    '\tfloat R;R;',
    '\tfloat r;r;',
    '\tfloat S;S;',
    '\tfloat s;s;',
    '\tfloat T;T;',
    '\tfloat t;t;',
    '\tfloat U;U;',
    '\tfloat u;u;',
    '\tfloat V;V;',
    '\tfloat v;v;',
    '\tfloat W;W;',
    '\tfloat w;w;',
    '\tfloat X;X;',
    '\tfloat x;x;',
    '\tfloat Y;Y;',
    '\tfloat y;y;',
    '\tfloat Z;Z;',
    '\tfloat z;z;',
    '\tfloat _;_;',
    '\tfloat +;+;',
    '\tfloat * ptr;&',
]

allowed = [
    '\tfloat * &_;',
    '\tfloat *ptr;',
]
