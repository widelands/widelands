#!/bin/bash

expected_lines=(
"/\*"
" \* Copyright( \(C\))? 20[0-9][0-9](-20[0-9][0-9])? by the Widelands Development Team"
" \*"
" \* This program is free software; you can redistribute it and/or"
" \* modify it under the terms of the GNU General Public License"
" \* as published by the Free Software Foundation; either version 2"
" \* of the License, or \(at your option\) any later version\."
" \*"
" \* This program is distributed in the hope that it will be useful,"
" \* but WITHOUT ANY WARRANTY; without even the implied warranty of"
" \* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\.  See the"
" \* GNU General Public License for more details\."
" \*"
" \* You should have received a copy of the GNU General Public License"
" \* along with this program; if not, see <https://www\.gnu\.org/licenses/>\."
)

err=0
n=0
for file in $(find $1 '(' -name '*.cc' -or -name '*.h' ')' -not -path '*third_party*')
do
  ((++n))
  printf "\r[%4d] Checking %-80s" $n "$file ..."
  exec 3<"$file"

  iserror=0
  for ((i=0; i < ${#expected_lines[@]}; ++i))
  do
    IFS= read line <&3
    result=$(grep -Po "${expected_lines[$i]}" <<< "$line" 2>/dev/null)
    if ! [ "$result" = "$line" ]
    then
      iserror=1
      break
    fi
  done

  if [ $iserror -eq 1 ]
  then
    printf "\nWrong copyright header in %s\n" "$file"
    ((++err))
  fi
done

echo
echo
if [ $err -eq 0 ]
then
  echo "All files are clean."
else
  echo "There were $err errors, please fix!"
  exit 1
fi
