#!/bin/bash

## This script will check out the current master and then pull new translations
## from Transifex.
## It then updates the developers/authors file.
## Afterwards, the catalogs will be updated.
## We also run utils/fix_formatting.py to periodically format the source code.
## Finally, the result will pushed to the master branch on GitHub
## and to Transifex.

# Exit as soon as any line in the bash script fails.
set -e

if [ -z "$1" ]
then
  push_target="https://github.com/widelands/widelands.git"
else
  push_target="$1"
fi

# Move up if we're not in the base directory.
if [ -d "../utils" ]; then
  pushd ..
fi

# Make sure 'utils/buildcat.py' is there.
if [ ! -f "utils/buildcat.py" ]; then
  echo "Unable to find 'utils/buildcat.py'."
  echo "Make sure you start this script from Widelands' base or utils directory.";
  exit 1
fi

# Ensure that our git is clean
rm -f po/*/*.pot.*~
rm -f po/*/*.po.*~

if [ -n "$(git status -s)" ]; then
  echo "git status must be empty to prevent accidental commits etc."
  git status
  exit 1
fi

# Checkout master and pull latest version
git checkout master
git pull "$push_target" master

# Double-check that it's clean
STATUS="$(git status)"
echo "${STATUS}"
if [[ "${STATUS}" != *"nothing to commit, working tree clean"* ]]; then
  echo "git status must be empty to prevent accidental commits etc."
  exit 1
fi

echo "Working tree is clean, continuing"

# Print all commands.
set -x

# Pull All translations from Transifex
# use git timestamps because the timestamps on disk are too new
tx pull -a --use-git-timestamps

if [ -n "$(git status -s)" ]; then
  # Stage and commit translations
  git add po/*/*.po data/i18n/locales/*.json xdg/translations/*.json || true
  git commit -m "Fetched translations."
fi

# -----------------------

# Update source catalogs
python3 utils/buildcat.py

undo_oneliner_diffs() {
  # Undo one-liner diffs of pure timestamps with no other content
  set +x
  git diff --numstat po/ | sed -En 's/^1\t1\t//p' | while IFS= read -r entry; do
    if ! git diff "$entry" | grep '^[+-][^+-]' | grep -qv '^[+-]"POT-Creation-Date:'
    then # no other diff line remaining
      echo "Skipping changes to $entry"
      git checkout "$entry"
    fi
  done
  set -x
}
undo_oneliner_diffs

if [ -n "$(git status -s)" ]; then
  # Only upload to Transifex if anything changed
  # Push source catalogs to Transifex
  tx push -s
  sleep 65 # wait for translation files to be updated
fi

# Pull All translations from Transifex
# (timestamps on disk are old enough this time)
tx pull -a

# Update authors file
if python3 utils/update_authors.py; then
  echo "Updated authors";
else
  echo "Failed updating authors";
  exit 1;
fi

# Update appdata
if python3 utils/update_appdata.py; then
  echo "Updated appdata";
else
  echo "Failed updating appdata";
  exit 1;
fi

# Update statistics
if python3 utils/update_translation_stats.py; then
  echo "Updated translation stats";
else
  echo "Failed to update translation stats";
  exit 1;
fi

# Fix formatting for Lua
python3 utils/fix_formatting.py --lua --dir data/i18n
python3 utils/fix_formatting.py --lua --dir data/txts

# Undo one-liner diffs of pure timestamps with no other content
undo_oneliner_diffs

if [ -z "$(git status -s)" ]; then
  echo "Run completed, nothing to commit."
else
  # Stage changes
  # - Translations
  git add po/*/*.po po/*/*.pot data/i18n/locales/*.json xdg/translations/*.json || true
  # - Authors
  git add data/txts/*.lua || true
  # - Locale data
  git add data/i18n/*.lua || true
  # - Appdata
  git add xdg/org.widelands.Widelands.appdata.xml xdg/org.widelands.Widelands.desktop || true
  # - Statistics
  git add data/i18n/translation_stats.conf || true

  # Commit
  git commit -m "Updated translations catalogs."
fi

# push fetched translations and updated catalogs
git push "$push_target" master
