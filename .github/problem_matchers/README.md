# Problem matchers

problem matchers for widelands to mark problems on GitHub

doc about problem matchers: https://github.com/actions/toolkit/blob/main/docs/problem-matchers.md

## file_line_column_msg.json
The output of several tools is almost compatible to [gcc](#ammaraskar/gcc-problem-matcher), but has no severity.
Therefore a similar matcher is included here.

Failure message it matches on:
```
/home/me/widelands/src/x/dump.h:18: Tabs are only allowed before the first non-whitespace character in line.
utils/validate_json.py:4:1: 'codecs' imported but unused
```

Used for tools:

- cmake/codecheck/CodeCheck.py in .github/workflows/build_codecheck.yaml

## ammaraskar/gcc-problem-matcher
This action is used for the problem matcher for gcc compiler and other tools.
See https://github.com/ammaraskar/gcc-problem-matcher

Failure message it matches on:
```
src/wlapplication_options.h:22:10: fatal error: map: No such file or directory
src/chat/wrong_file.cc:22: warning: xxx
```

Used for tools:
