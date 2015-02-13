#!/bin/bash

## This script will pull new translations from Transifex into local trunk
## and fix the line breaks.
## Afterwards, the catalogs will be updated and the result pushed to
## trunk on Launchpad.

set -e # Exit as soon as any line in the bash script fails.

# Move up if we're not in the base directory.
if [ -d "../utils" ]; then
	pushd ..
fi

# Make sure 'utils/buildcat.py' and 'utils/remove_lf_in_translations.py' are there.
if [ ! -f "utils/buildcat.py" -o ! -f "utils/remove_lf_in_translations.py" ]; then
	echo "Unable to find 'utils/buildcat.py' or 'utils/remove_lf_in_translations.py'."
	echo "Make sure you start this script from Widelands' base or utils directory.";
	exit;
fi

# Make sure we have a local trunk branch.
if [ ! -d "../trunk" ]; then
	echo "Please branch lp:widelands into ../trunk";
	exit;
fi

set -x # Print all commands.

# Pull translations from Transifex into local trunk and add new translation files
pushd ../trunk && bzr pull
tx pull -a
bzr add po/*/*.po
bzr commit -m "Merged translations."

# Fix line breaks.
utils/remove_lf_in_translations.py
bzr commit -m "Fixed LF in translations." || true

# Update catalogues.
utils/buildcat.py
bzr commit -m "Updated catalogues."
bzr push lp:widelands

# Push catalogues to Transifex
tx push -s
