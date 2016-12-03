#!/bin/bash

## This script will pull new translations from Transifex into local trunk
## and fix the line breaks.
## It then updates the developers/authors file.
## Afterwards, the catalogs will be updated and the result pushed to
## trunk on Launchpad.

# Exit as soon as any line in the bash script fails.
set -e

# Move up if we're not in the base directory.
if [ -d "../utils" ]; then
	pushd ..
fi

# Make sure 'utils/buildcat.py' and 'utils/remove_lf_in_translations.py' are there.
if [ ! -f "utils/buildcat.py" -o ! -f "utils/remove_lf_in_translations.py" ]; then
	echo "Unable to find 'utils/buildcat.py' or 'utils/remove_lf_in_translations.py'."
	echo "Make sure you start this script from Widelands' base or utils directory.";
	exit 1;
fi

# Make sure we have a local trunk branch.
PARENT=$(bzr config parent_location)
if [[ "$PARENT" != "bzr+ssh://bazaar.launchpad.net/~widelands-dev/widelands/trunk/" &&
      "$PARENT" != "bzr+ssh://bazaar.launchpad.net/+branch/widelands/" ]]; then
	echo "The current bzr branch is not trunk.";
	exit 1;
fi

# Print all commands.
set -x

# Pull translations from Transifex into local trunk and add new translation files
bzr pull
tx pull -a
bzr add po/*/*.po data/i18n/locales/*.json debian/translations/*.json || true

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

# Fix formatting
utils/fix_formatting.py
if [ $? -eq 0 ]
then
  echo "Fixed formatting";
else
  echo "Failed to fix formatting";
  exit 1;
fi

# Fix line breaks.
# TODO(GunChleoc): We hope that Transifex will fix these already.
# This script can be removed if we don't get any errors in the future.
# utils/remove_lf_in_translations.py

# Update catalogues.
utils/buildcat.py
bzr commit -m "Fetched translations and updated catalogues."
bzr push lp:widelands

# Push catalogues to Transifex
#tx push -s
