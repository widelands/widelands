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
if [ ! -f "utils/buildcat.py" -o ! -f "utils/remove_lf_in_translations.py" ]; then
  echo "Unable to find 'utils/buildcat.py' or 'utils/remove_lf_in_translations.py'."
  echo "Make sure you start this script from Widelands' base or utils directory.";
  exit 1;
fi

# Make sure that the output directory is there
if [ ! -d "po_validation" ]; then
  echo "Creating directory 'po_validation'"
  mkdir po_validation
fi

echo "Running i18nspector. This can take a while."

# We want a log of all errors.
i18nspector po/*/*.po | grep "E:" > po_validation/i18nspector-errors.log

# We don't care about all warnings, so we filter some out.
i18nspector po/*/*.po | grep "W:" | grep -v "invalid-last-translator" | grep -v boilerplate > po_validation/i18nspector-warnings.log


# Takes type of check as an argument.
# Makes a subdirectory for the argument if necessary,
# then runs pofilter with the check.
function run_pofilter() {
  if [ ! -d "po_validation/$1/" ]; then
    mkdir po_validation/$1/
  fi
  echo "Running pofilter for '$1'"
  pofilter -t $1 -i po/ -o po_validation/$1/
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
run_pofilter "startwhitespace"
run_pofilter "endwhitespace"
run_pofilter "startcaps"
run_pofilter "numbers"

# We only run the options check on the command line help
if [ ! -d "po_validation/options/" ]; then
  echo "Creating directory 'options'"
  mkdir po_validation/options/
fi
echo "Running pofilter for 'options'"
pofilter -t options -i po/widelands_console/ -o po_validation/options/

echo "Cleaning up empty directories"
for dir in po_validation/*/
do
  dir=${dir%*/}
  echo "- Cleaning up ${dir##*/}"
    if [ ! "$(find po_validation/${dir##*/}/ -mindepth 1 -maxdepth 1 -type d -printf . | wc -c)" -eq 0 ]; then
    for subdir in po_validation/${dir##*/}/*/
    do
      subdir=${subdir%*/}
      if [ ! "$(ls -A po_validation/${dir##*/}/${subdir##*/})" ]; then
        rmdir po_validation/${dir##*/}/${subdir##*/}
      fi
    done
  fi
  if [ ! "$(ls -A po_validation/${dir##*/})" ]; then
    rmdir po_validation/${dir##*/}
  fi
done

echo "Done"
