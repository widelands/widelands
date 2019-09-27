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

STATUS="$(git status)"
echo "${STATUS}"
if [[ "${STATUS}" != *"nothing to commit, working tree clean"* ]]; then
  echo "git status must be empty to prevent accidental commits etc."
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
tx pull -a

# Update authors file
utils/update_authors.py
if [ $? -eq 0 ]
then
  echo "Updated authors";

else
  echo "Failed updating authors";
  exit 1;
fi

# Update appdata
utils/update_appdata.py
if [ $? -eq 0 ]
then
  echo "Updated appdata";
else
  echo "Failed updating appdata";
  exit 1;
fi

# Update catalogs
utils/buildcat.py

# Update statistics
utils/update_translation_stats.py
if [ $? -eq 0 ]
then
  echo "Updated translation stats";
else
  echo "Failed to update translation stats";
  exit 1;
fi

# Fix formatting for C++, Lua & Python
python utils/fix_formatting.py

# Stage changes
# - Translations
git add po/*/*.po po/*/*.pot data/i18n/locales/*.json debian/translations/*.json || true
# - Authors
git add data/txts/*.lua || true
# - Appdata
git add debian/widelands.appdata.xml debian/org.widelands.widelands.desktop || true
# - Statistics
git add data/i18n/translation_stats.conf || true

# Commit and push.
git commit -m "Fetched translations and updated catalogs."
git push https://github.com/widelands/widelands.git master

# Push catalogs to Transifex
tx push -s
