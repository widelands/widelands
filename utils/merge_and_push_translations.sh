## This script will fix translations in the translation branch
## and then push the fix to the translations branch on Launchpad.
## Afterwards, the translations branch will be merged into trunk,
## the catalogs be updated and the result pushed to trunk on Launchpad.

# Fix LF in translation branch.
cd ../translations && bzr pull
utils/remove_lf_in_translations.py
bzr add po
bzr commit -m "Fixed LF in translations." || true
bzr push lp:~widelands-dev/widelands/translations

# Merge translations.
cd ../trunk && bzr pull
bzr merge lp:~widelands-dev/widelands/translations
bzr commit -m "Merged translations."

# Update catalogues.
utils/buildcat.py
bzr add po
bzr commit -m "Updated catalogues."
bzr push lp:widelands
