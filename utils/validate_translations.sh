#!/bin/bash

## This script will validate all po files, using the i18nspector and
## translate-toolkit packages.

# Exit as soon as any line in the bash script fails.
set -e

# Move up if we're not in the base directory.
if [ -d "../utils" ]; then
  pushd ..
fi

# Double-check that we're in the correct directory.
if [ ! -f "utils/buildcat.py" ]; then
  echo "Unable to find 'utils/buildcat.py'."
  echo "Make sure you start this script from Widelands' base or utils directory.";
  exit 1;
fi

# Make sure that the output directories are there and empty
echo "Creating directories in 'po_validation'"
if [ ! -d "po_validation" ]; then
  mkdir po_validation
fi
if [ ! -d "po_validation/maintainers" ]; then
  mkdir po_validation/maintainers
else
  rm -rf po_validation/maintainers/*
fi
if [ ! -d "po_validation/translators" ]; then
  mkdir po_validation/translators
else
  rm -rf po_validation/translators/*
fi

echo "Running i18nspector. This can take a while."

# We want a log of all errors.
i18nspector data/i18n/translations/*/*.po \
  | grep "E:" \
  > po_validation/maintainers/i18nspector-errors.log

# We don't care about all warnings, so we filter some out.
i18nspector data/i18n/translations/*/*.po \
  | grep "W:" \
  | grep -v "invalid-last-translator" \
  | grep -v boilerplate \
  > po_validation/maintainers/i18nspector-warnings.log


# Takes type of check as an argument.
# Makes a subdirectory for the argument if necessary,
# then runs pofilter with the check.
function run_pofilter() {
  if [ ! -d "po_validation/maintainers/$1/" ]; then
    mkdir po_validation/maintainers/$1/
  fi
  echo "Running pofilter for '$1'"
  pofilter -t $1 -i data/i18n/translations/ -o po_validation/maintainers/$1/
}

# These checks are critical
run_pofilter "printf"
run_pofilter "nplurals"

# These checks are important
run_pofilter "emails"
run_pofilter "filepaths"
run_pofilter "validchars"
run_pofilter "variables"
run_pofilter "urls"

# These checks are nice to have
run_pofilter "doublespacing"
run_pofilter "doublewords"
run_pofilter "startwhitespace"
run_pofilter "endwhitespace"
run_pofilter "startpunc"
run_pofilter "endpunc"
run_pofilter "startcaps"
run_pofilter "numbers"

# We only run the options check on the command line help
if [ ! -d "po_validation/maintainers/options/" ]; then
  echo "Creating directory 'options'"
  mkdir po_validation/maintainers/options/
fi
echo "Running pofilter for 'options'"
pofilter -t options -i data/i18n/translations/widelands_console/ -o po_validation/maintainers/options/

echo "Processing directories"
for dir in po_validation/maintainers/*/
do
  dir=${dir%*/}
  echo "- ${dir##*/}"
    if [ ! "$(find po_validation/maintainers/${dir##*/}/ -mindepth 1 -maxdepth 1 -type d -printf . | wc -c)" -eq 0 ]; then
    for subdir in po_validation/maintainers/${dir##*/}/*/
    do
      subdir=${subdir%*/}
      if [ ! "$(ls -A po_validation/maintainers/${dir##*/}/${subdir##*/})" ]; then
        # Delete empty directories
        rmdir po_validation/maintainers/${dir##*/}/${subdir##*/}
      else
        # Copy files to translators' view
        category=$(basename "$dir")
        textdomain=$(basename "$subdir")
        for localepath in $(find $subdir -maxdepth 2 -type f)
        do
          locale=$(basename "${localepath%.*}")
          targetpath="po_validation/translators/$locale/$textdomain/$category.po"
          if [ ! -d "po_validation/translators/$locale/" ]; then
              mkdir po_validation/translators/$locale/
          fi
          if [ ! -d "po_validation/translators/$locale/$textdomain/" ]; then
              mkdir po_validation/translators/$locale/$textdomain/
          fi
          cp $localepath $targetpath
        done
      fi
    done
  fi
done

echo "Done"
