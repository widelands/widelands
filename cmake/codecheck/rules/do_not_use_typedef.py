#!/usr/bin/python

strip_comments_and_strings = True

def evaluate_matches(lines, fn):
   errors = []

   for lineno, line in enumerate(lines):
      if line.count("typedef"):
         errors.append((fn, lineno+1, "Do not use \"typedef\". Use \"using\" instead."))

   return errors
# /end evaluate_matches

forbidden = [
    "typedef uint8_t MyType"
]

allowed = [
    "using MyType = uint8_t"
]
