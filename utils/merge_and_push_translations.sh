#!/bin/bash

## This script will fix translations in the translation branch
## and then push the fix to the translations branch on Launchpad.
## Afterwards, the translations branch will be merged into trunk,
## the catalogs be updated and the result pushed to trunk on Launchpad.

set -e # Exit as soon as any line in the bash script fails.

# Move up if we're not in the base directory.
if [ -d "../utils" ]; then
	pushd ..
	EXTRAPUSH=1 # We will need an extra popd on the bottom.
fi

# Make sure 'utils/buildcat.py' and 'utils/remove_lf_in_translations.py' are there.
if [ ! -f "utils/buildcat.py" -o ! -f "utils/remove_lf_in_translations.py" ]; then
	echo "Unable to find 'utils/buildcat.py' or 'utils/remove_lf_in_translations.py'."
	echo "Make sure you start this script from Widelands' base or utils directory.";
	exit;
fi

# Make sure we have the needed branches.
if [ ! -d "../trunk" ]; then
	echo "Please branch lp:widelands into ../trunk";
	exit;
fi

if [ ! -d "../translations" ]; then
	echo "Please branch lp:~widelands-dev/widelands/translations into ../translations";
	exit;
fi

set -x # Print all commands.

# Fix LF in translation branch.
pushd ../translations && bzr pull
utils/remove_lf_in_translations.py
bzr commit -m "Fixed LF in translations." || true
bzr push lp:~widelands-dev/widelands/translations

# Merge translations.
pushd ../trunk && bzr pull
bzr merge lp:~widelands-dev/widelands/translations
bzr commit -m "Merged translations."

# Update catalogues.
utils/buildcat.py
bzr commit -m "Updated catalogues."
bzr push lp:widelands

set +x # Stop printing all commands.

popd
popd

if [ $EXTRAPUSH ]; then
   popd
fi

echo "Finished updating translations."
