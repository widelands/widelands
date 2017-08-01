#!/usr/bin/python

error_msg = 'Do not use private inheritance. Favor composition instead.'

strip_comments_and_strings = True
regexp = r':.*private '

allowed = [
    'y = private palu',
    ' class Foo : public A',
]

forbidden = [
    ' class Foo : private A',
    ' class Foo : public A, private B',
]
