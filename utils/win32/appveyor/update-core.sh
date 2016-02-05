#!/usr/bin/env bash
#
#   update-core - properly update core packages for MSYS2
#
#   Copyright (c) 2006-2015 Pacman Development Team <pacman-dev@archlinux.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

shopt -s extglob

# gettext initialization
export TEXTDOMAIN='pacman-scripts'
export TEXTDOMAINDIR='/usr/share/locale'

declare -r myver='4.2.1-463-g28da-dirty'
declare -r confdir='/etc'
declare -r CRITICAL_PACKAGES="bash pacman msys2-runtime"
declare -r OPTIONAL_PACKAGES="msys2-runtime-devel"

# check if messages are to be printed using color
unset ALL_OFF BOLD BLUE GREEN RED YELLOW
if [[ -t 2 && ! $USE_COLOR = "n" ]]; then
	# prefer terminal safe colored and bold text when tput is supported
	if tput setaf 0 &>/dev/null; then
		ALL_OFF="$(tput sgr0)"
		BOLD="$(tput bold)"
		BLUE="${BOLD}$(tput setaf 4)"
		GREEN="${BOLD}$(tput setaf 2)"
		RED="${BOLD}$(tput setaf 1)"
		YELLOW="${BOLD}$(tput setaf 3)"
	else
		ALL_OFF="\e[1;0m"
		BOLD="\e[1;1m"
		BLUE="${BOLD}\e[1;34m"
		GREEN="${BOLD}\e[1;32m"
		RED="${BOLD}\e[1;31m"
		YELLOW="${BOLD}\e[1;33m"
	fi
fi
readonly ALL_OFF BOLD BLUE GREEN RED YELLOW

plain() {
	(( QUIET )) && return
	local mesg=$1; shift
	printf "${BOLD}    ${mesg}${ALL_OFF}\n" "$@" >&1
}

msg() {
	(( QUIET )) && return
	local mesg=$1; shift
	printf "${GREEN}==>${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "$@" >&1
}

msg2() {
	(( QUIET )) && return
	local mesg=$1; shift
	printf "${BLUE}  ->${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "$@" >&1
}

ask() {
	local mesg=$1; shift
	printf "${BLUE}::${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}" "$@" >&1
}

warning() {
	local mesg=$1; shift
	printf "${YELLOW}==> $(gettext "WARNING:")${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "$@" >&2
}

error() {
	local mesg=$1; shift
	printf "${RED}==> $(gettext "ERROR:")${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "$@" >&2
}

if [[ -n "$MSYSTEM" ]]; then
  readonly -a UTILS_NAME=('bsdtar'
                          'bzip2'
                          'bzr'
                          'cat'
                          'ccache'
                          'distcc'
                          'git'
                          'gpg'
                          'gzip'
                          'hg'
                          'lzip'
                          'lzop'
                          'openssl'
                          'svn'
                          'tput'
                          'uncompress'
                          'upx'
                          'xargs'
                          'xz'
                         )

  for wrapper in ${UTILS_NAME[@]}; do
    eval "
    ${wrapper}"'() {
      local UTILS_PATH="/usr/bin/"
      if ! type -p ${UTILS_PATH}${FUNCNAME[0]} >/dev/null; then
        error "$(gettext "Cannot find the %s binary required for makepkg.")" "${UTILS_PATH}${FUNCNAME[0]}"
        exit 1
      fi
      ${UTILS_PATH}${FUNCNAME[0]} "$@"
    }'
  done
fi


# print usage instructions
usage() {
	printf "update-core (pacman) %s\n" ${myver}
	echo
}

version() {
	cmd=${0##*/}
	printf "%s (pacman) %s\n\n" "$cmd" "$myver"
	printf -- "$(gettext "\
Copyright (c) 2006-2015 Pacman Development Team <pacman-dev@archlinux.org>\n\n\
This is free software; see the source for copying conditions.\n\
There is NO WARRANTY, to the extent permitted by law.\n")"
}


run_pacman() {
	local cmd
	cmd=("$PACMAN_PATH" "$@")

	"${cmd[@]}"
}


# PROGRAM START

# determine whether we have gettext; make it a no-op if we do not
if ! type -p /usr/bin/gettext &>/dev/null; then
	gettext() {
		printf "%s\n" "$@"
	}
else
	gettext() {
		/usr/bin/gettext "$@"
	}
fi

case $1 in
	-h|--help) usage; exit 0;;
	-V|--version) version; exit 0;;
esac

# set pacman command if not already defined
PACMAN=${PACMAN:-pacman}
# save full path to command as PATH may change when sourcing /etc/profile
PACMAN_PATH=$(type -P $PACMAN)

msg "Update package databases..."
if ! run_pacman -Sy; then
  error "$(gettext "'%s' failed to update package databases.")" "$PACMAN"
  exit 1 # TODO: error code
fi

msg "Checking if there are critical packages to upgrade."
if ! run_pacman -Qu ${CRITICAL_PACKAGES}; then
  msg "No updates for core packages."
else
  msg "Core packages require updating."
  msg "Please close all other MSYS2 derived windows (e.g. terminal"
  msg "windows, Bash sessions, etc) before proceeding."
  warning "When the update has completed, you MUST close this MSYS2 window"
  warning "(use Alt-F4 or red [ X ], etc.), rather than 'exit'!!!"
  read -p "Press [Enter] key when ready to start update..."
  export PS1='Please close this window.'
  export PROMPT_COMMAND=
  msg "Updating core packages..."
  if ! run_pacman -S --noconfirm --needed ${CRITICAL_PACKAGES} ${OPTIONAL_PACKAGES}; then
    error "$(gettext "'%s' failed to update core packages.")" "$PACMAN"
    exit 1 # TODO: error code
  fi
fi

# vim: set noet:
