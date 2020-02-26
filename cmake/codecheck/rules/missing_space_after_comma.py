#!/usr/bin/python


"""This catches a comma that is followed by another character than a space, for
example "a,b,c", which should be "a, b, c".

It is a little complex because it should not catch commas in string
constants or character constants.
"""

error_msg = "A space is mandatory after ','."

strip_comments_and_strings = True
regexp = r""",[^\s]"""


forbidden = [
    "do_something('\t','u')",
    'do_something("\t","u")',

    # Eriks tests
    # missing_space_after_comma.cc
    'float __,_,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;',
    'float ____,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z;',
    'float XX = ff(_,0,1,2,3,4);',
    'float YY = ff(_,5,6,7,8,9);',
]

allowed = [
    '''do_something(",", ',')''',
    '/* We,are inside a comment,*/',
    '// This is,a comment too',

    # Eriks tests
    # missing_space_after_comma.cc
    'float ff(float, float, float, float, float, float);',
]
