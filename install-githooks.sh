#!/bin/sh

## Creates symbolic links for our githooks

if [ -f .git/hooks/pre-commit ]; then
  rm .git/hooks/pre-commit
fi

ln -s ../../utils/githooks/pre-commit.sh .git/hooks/pre-commit
