#!/bin/bash

## Creates symbolic links for our githooks

rm .git/hooks/pre-commit || true
ln -s ../../utils/githooks/pre-commit.sh .git/hooks/pre-commit
