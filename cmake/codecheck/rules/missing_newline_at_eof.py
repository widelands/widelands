#!/usr/bin/python -tt

class EvalMatches( object ):
    def __call__(self, lines,fn):
        errors = []
        if not len(lines) or not len(lines[-1]) or lines[-1][-1] != '\n':
                errors.append( (fn, len(lines), "Missing newline at eof!"))
        return errors


evaluate_matches = EvalMatches()

forbidden = [
    "jlhsdfjh",
    "ksdhdsf\nkjshfkjhdsf",
    "",
]

allowed = [
    "jlhsdfjh\n",
    "ksdhdsf\nkjshfkjhdsf\n",
]
