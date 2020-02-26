#!/usr/bin/python


"""
Comments should all start with a blank space
"""


strip_comments_and_strings = False
regexp = r"""((^|\s)//[^\s/<!])|((^|\s)//<[^\s])|((^|\s)//!<[^\s])"""

def is_space(s, index):
    if index >= len(s):
        return True
    return s[index] == " "

def evaluate_matches(lines, fn):
   errors = []
   for lineno, line in enumerate(lines):
      line = line.strip()
      if not line.startswith('//'):
          continue
      if line.startswith('///!<') and is_space(line, 5):
          continue
      if line.startswith('//!<') and is_space(line, 4):
          continue
      if line.startswith('///<') and is_space(line, 4):
          continue
      if line.startswith('///') and is_space(line, 3):
          continue
      if line.startswith('//<') and is_space(line, 3):
          continue
      if line.startswith('//') and is_space(line, 2):
          continue
      errors.append((fn, lineno+1, "Comments need to start with a space."))
   return errors


forbidden = [
    '//spaceless comment',
    '///spaceless comment',
    '//<spaceless comment',
    '///<spaceless comment',
]

allowed = [
    '// nifty comment',
    '/// nifty comment',
    '//< we have a lot of those',
    '///< we have a lot of those',
    '//!< and some of those',
    '///!< and some of those',
]
