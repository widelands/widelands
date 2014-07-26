#!/usr/bin/python

import re

def evaluate_matches(lines, fn):
   errors = []

   for lineno, line in enumerate(lines):
      errorfound = False

      # Allow only "TODO" as keyword
      if line.count("FIXME"):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not use\"FIXME\"."))

      elif re.compile("/\s*BUG").search(line):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not use\"BUG\"."))

      elif re.compile("[*]\s*BUG").search(line):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not use\"BUG\" or put TODOs in Doxygen comments."))

      # No dogygen
      elif line.count("\TODO"):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not put TODOs in Doxygen comments."))

      elif line.count("\todo"):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not put TODOs in Doxygen comments."))

      elif re.compile("[*]\s*TODO").search(line):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not put TODOs in Doxygen comments."))

      elif re.compile("///\s*TODO").search(line):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not put TODOs in Doxygen comments."))

      elif re.compile("//!\s*TODO").search(line):
         errorfound = True
         errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not put TODOs in Doxygen comments."))

      # Proper formatting of TODO = TODO(username): <msg>
      if not errorfound:
         finderror = line.rsplit("TODO")
         if len(finderror) > 1:
            for (counter, temp) in enumerate(finderror):
               if counter > 0:
                 if not temp.startswith("("):
                    errorfound = True
                    errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\"."))
                 else:
                    findend = temp.split("):")
                    if len(findend) < 2:
                       errorfound = True
                       errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\"."))
                    # Prevent empty messages
                    elif len(findend) == 2 and not findend[1].lstrip():
                       errorfound = True
                       errors.append((fn, lineno+1, "Use \"TODO(username): <msg>\". Do not add empty TODO comments."))
   # /end for
   return errors
# /end evaluate_matches

forbidden = [
    "// FIXME this is a todo comment",
    "// BUG this is a todo comment",
    "// TODO this is a todo comment",
    "// TODO: This is a todo comment",
    "* TODO: This is a todo comment",
    "/// TODO: This is a todo comment",
    "//! TODO: This is a todo comment",
    "\TODO: This is a todo comment",
    "\\\todo: This is a todo comment"
]

allowed = [
    "// TODO(<username>): This is a todo comment"
]
