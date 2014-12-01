## This script will fix translations in the translation branch
## and then push the fix to the translations branch on Launchpad.
## Afterwards, the translations branch will be merged into trunk,
## the catalogs be updated and the result pushed to trunk on Launchpad.

set -e # Exit as soon as any line in the bash script fails.

# Move up if we're in the utils directory.
TEST=`(ls | grep "buildcat.py")`
if [ $TEST = "buildcat.py" ]; then
	echo "Leaving the utils dir"
	cd ..
fi

echo "We are in the following directory:"
pwd

# Make sure 'utils/buildcat.py' is there.
TEST=`(ls utils | grep "buildcat.py")`
if [ $TEST != "buildcat.py" ]; then
	echo "Unable to find 'utils/buildcat.py'."
	echo "Make sure you start this script from Widelands' base or utils directory.";
	exit;
fi

# Make sure 'utils/remove_lf_in_translations.py' is there.
TEST=`(ls utils | grep "remove_lf_in_translations.py")`
if [ $TEST != "remove_lf_in_translations.py" ]; then
	echo "Unable to find 'utils/remove_lf_in_translations.py'."
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
cd ../translations && bzr pull
utils/remove_lf_in_translations.py
bzr commit -m "Fixed LF in translations." || true
bzr push lp:~widelands-dev/widelands/translations

# Merge translations.
cd ../trunk && bzr pull
bzr merge lp:~widelands-dev/widelands/translations
bzr commit -m "Merged translations."

# Update catalogues.
utils/buildcat.py
bzr commit -m "Updated catalogues."
bzr push lp:widelands
