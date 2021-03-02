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
git pull https://github.com/widelands/widelands.git master

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

# Pull translations from Transifex
# tx pull -a
# Force-pull translations because some would get skipped accidentally
tx pull -fa

# Update authors file
python3 utils/update_authors.py
if [ $? -eq 0 ] ; then
  echo "Updated authors";
else
  echo "Failed updating authors";
  exit 1;
fi

# Update appdata
python3 utils/update_appdata.py
if [ $? -eq 0 ] ; then
  echo "Updated appdata";
else
  echo "Failed updating appdata";
  exit 1;
fi

# Update catalogs
python3 utils/buildcat.py

# Update statistics
python3 utils/update_translation_stats.py
if [ $? -eq 0 ] ; then
  echo "Updated translation stats";
else
  echo "Failed to update translation stats";
  exit 1;
fi

# Fix formatting for Lua
python3 utils/fix_formatting.py --lua --dir data/i18n
python3 utils/fix_formatting.py --lua --dir data/txts

# Undo one-liner diffs in po directory - these are pure timestamps with no other content
set +x
for line in $(git diff --numstat po); do
  row=($(echo $line | tr "\t" "\n"))
  if [ ${#row[@]} -eq 3 ] ; then
    if [ ${row[0]} -eq 1 -a ${row[1]} -eq 1 ] ; then
      echo "Skipping changes to ${row[2]}"
      git checkout ${row[2]}
    fi
  fi
done
set -x

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

# Commit and push.
git commit -m "Fetched translations and updated catalogs."
git push https://github.com/widelands/widelands.git master

# Push catalogs to Transifex
tx push -s
