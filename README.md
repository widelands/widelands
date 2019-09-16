[![Build status](https://api.travis-ci.org/widelands/widelands.svg?master)](https://travis-ci.org/widelands/widelands)

### Registering the `git` hook [1]

Run at a terminal:

```sh
$ ./git-pre-commit-format install
```

This will add a hook (in `.git/hooks/`) which is run every time you commit.

###Using the pre-commit hook

After registering the git hook, you don't need to do anything except for committing your changes.

Every time you commit, the hook will check if your code matches the coding standard. If it doesn't, you get asked what to do and you can decide to:

* Apply the fixes automatically (only to the code you are actually committing, not to unstaged code).
* Commit anyway.
* Abort the commit so you can fix the problem manually.

Note that, if you use `git` through a GUI or some other tool (i.e. not directly on the command line), the script will fail to get your input.
In this case disable the interactive behaviour with:

```sh
$ git config hooks.clangFormatDiffInteractive false
```

# Links
[1] [barisione/clang-format-hooks]( https://github.com/barisione/clang-format-hooks/blob/master/README.md#registering-the-git-hook)