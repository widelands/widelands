#!/bin/zsh

# Copyright (c) 2021 Imre Horvath <imi [dot] horvath [at] gmail [dot] com>
# MIT License
# Should work with /bin/sh or /bin/bash too.
# Inspired by:
# https://github.com/renard/emacs-build-macosx/blob/master/build-emacs
# https://github.com/trojanfoe/xcodedevtools/blob/master/copy_dylibs.py

set -e

relpath="../Frameworks"  # dylibs go to @executable_path/../Frameworks by default

usage_text="Usage: $(basename "$0") [-h] [-l relative_path] path_to_app

-h Show this usage description and exit.

-l relative_path
   Specifies, where to put the bundled dylibs relative to the executable.
   When omitted, defaults to: \"$relpath\"."

usage() {
    echo "$usage_text" 1>&2
    exit ${1:-1}
}

while getopts ':hl:' opt; do
    case "$opt" in
	h)
	    usage 0
	    ;;
	l)
	    relpath="$OPTARG"
	    ;;
	\?)
	    echo "Invalid option -$OPTARG" 1>&2
	    usage
	    ;;
    esac
done
shift $((OPTIND-1))

[[ $# -eq 1 ]] || usage

bundlepath="$1"
libdir="$bundlepath/Contents/MacOS/$relpath"

list_dylibs() {
    local file="$1"
    echo $(otool -L "$file" |\
	       sed -n 's/^[[:space:]]*\(.*\) (.*$/\1/p' |\
	       sed -e '/^\/System/d' -e '/^\/usr\/lib/d' -e '/^@/d')
}

process_dylibs() {
    local dylibs="$1"
    local file="$2"
    local dylib
    local name
    local dest
    for dylib in $(echo $dylibs); do
	name="$(basename $dylib)"
	dest="$libdir/$name"
	if [[ ! -e $dest ]]; then
	    cp "$dylib" "$dest"
	    chmod 644 "$dest"
	    install_name_tool -id "@rpath/$name" "$dest"
	    process_dylibs "$(list_dylibs $dest)" "$dest"
	fi
	install_name_tool -change "$dylib" "@rpath/$name" "$file"
    done
}

process_executable() {
    local executable="$1"
    local dylibs="$(list_dylibs "$executable")"
    if [[ -n $dylibs ]]; then
	mkdir -p "$libdir"
	process_dylibs "$dylibs" "$executable"
	install_name_tool -add_rpath "@executable_path/$relpath" "$executable"
    fi
}

if [[ $(basename "$bundlepath") =~ [.]app$ ]] &&
       [[ -f $bundlepath/Contents/Info.plist ]] &&
       [[ -d $bundlepath/Contents/MacOS ]]; then
    find "$bundlepath/Contents/MacOS" -type f -perm +111 -print0 | while IFS= read -r -d '' executable; do
	if [[ $(file "$executable") =~ Mach-O.*executable ]]; then
	    process_executable "$executable"
	fi
    done
else
    echo "\"$bundlepath\" does not appear to be an application bundle." 1>&2
    exit 1
fi

exit 0
