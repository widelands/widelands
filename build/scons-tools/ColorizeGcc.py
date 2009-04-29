#!/usr/bin/env python -tt
# encoding: utf-8
#
# File: build/scons-tools/ColorizeGcc.py
#
# Created by Holger Rapp on 2009-04-29.
# Copyright (c) 2009 HolgerRapp@gmx.net. All rights reserved.
#
# Last Modified: $Date$
#

"""
====================================================
class GccColorizer, knows how to colorize gcc output.
it doesnt use scons at all, and can be freely used. it is based on the 
colorgcc.pl script

This class was taken from a Post by Aviad Rozenhek (avia...@mobileye.com)
to the org.tigris.scons.dev mailing list on the 14. Dec 2006

I modified it slightliy
====================================================
"""

ansicolor = {
    "default" : '\033[0m',

    "black" : '\033[30m',
    "red" : '\033[31m',
    "green" : '\033[32m',
    "yellow" : '\033[33m',
    "blue" : '\033[34m',
    "magenta" : '\033[35m',
    "purple" : '\033[35m',
    "cyan" : '\033[36m',
    "white" : '\033[37m',

    "reset" : '\033[0;0m',
    "bold" : '\033[1m',
        
    "blackbg" : '\033[40m',
    "redbg" : '\033[41m',
    "greenbg" : '\033[42m',
    "yellowbg" : '\033[43m',
    "bluebg" : '\033[44m',
    "magentabg" : '\033[45m',
    "cyanbg" : '\033[46m',
    "whitebg" : '\033[47m',
}
def esc_ansicolor( conf ):
    color = ''.join( [ ansicolor[i] for i in conf.split(';') ] )

    return color

import re

class GccColorizer:
    def __init__(self):
        self.__color_names = {
            'reset' : 'default',

            'srcColor'  : 'bold;purple',
            'introColor': 'bold;cyan',

            'warningFileNameColor': 'green',
            'warningNumberColor'    : 'cyan',
            'warningMessageColor'    : 'bold;yellow',

            'errorFileNameColor': 'green',
            'errorNumberColor'    : 'green',
            'errorMessageColor'    : 'bold;red',
        }

        # convert the name to ansi-color representations
        self.__colors = {}
        for color in self.__color_names.keys():
            self.__colors [color] = esc_ansicolor (self.__color_names [color])

        flags = re.DOTALL | re.MULTILINE

        # compile a regular expression that captures the
        # source qouting of gcc
        self.__src_regex = re.compile (r"\`(.*?)\'", flags)

        # compile a regular expression that captures
        # filename:lineno:message format
        self.__file_line_msg_regex = re.compile(r"^([^:]*?)\s*:([0-9:]+):( .*)$", flags)

        # filename:message:
        self.__file_msg_regex= re.compile (r"^(.*?):(.+)[:,]$", flags)

        # compile a regular expression that captures warnings
        self.__warning_regex = re.compile (r"\s+warning:.*", flags)

        # comile a regex for wrapped message lines
        # wrap lines start with 3 spaces
        self.__wrap_regex = re.compile (r"^[ ]{3}[^ ]", flags)

        #buffer where we store lines until they become full message
        self.__buffer = ""

        # lists of error and warnings
        self.__errors = []
        self.__warnings = []

    def colorize_line (self, line):
        """colorize a single gcc message.
        if the message spans more than one line (has wraps) then
        the message is not printed until the complete message has been 
        retrieved."""
        if self.__wrap_regex.match (line):
            #self.__buffer += '\n'
            self.__buffer += line
        else:
            self.colorize_message (self.__buffer)
            self.__buffer = line

    def warnings (self):
        """returns a list of the warnings encountered."""
        return self.__warnings

    def errors (self):
        """returns a list of the errors encountered."""
        return self.__errors

    def reset (self):
        """resets the errors and warnings counters. removes any leftovers in the buffer."""
        self.__buffer = ""
        self.__errors = self.__warnings = []

    def colorize (self, lines):
        """ Colorize the output from the compiler.  'lines' is an array of the lines of output from the compiler."""

        if isinstance (lines, str):
            # convenience:
            # lines is a string, so we convert it to a list
            # by splitting the lines
            lines = lines.splitlines ()

        for line in lines:
            self.colorize_line (line)

        self.finish ()

    def finish (self):
        """call this method after all lines, in order to print the last buffer."""
        self.colorize_line ("")

    def colorize_message(self, message):
        """Colorizes and prints a full gcc message"""
        if (message == None or len(message)==0):
            return

        match = self.__file_line_msg_regex.match (message)
        if match:
            filename= match.group (1)
            lineno = match.group (2)
            msg = match.group (3)

            if self.__warning_regex.match (msg):
                # Warning
                filename_str = "%s%s%s" % (self.__colors["warningFileNameColor"], filename, self.__colors["reset"])
                line_str = "%s%s%s" % (self.__colors["warningNumberColor"], lineno, self.__colors["reset"])
                msg = self.__srcscan__(msg, self.__colors["warningMessageColor"])
               
                # add this message to the list of warnings
                self.__warnings.append (message)
            else:
                # Error
                filename_str = "%s%s%s" % (self.__colors["errorFileNameColor"], filename, self.__colors["reset"])
                line_str = "%s%s%s" % (self.__colors["errorNumberColor"], lineno, self.__colors["reset"])
                msg = self.__srcscan__(msg, self.__colors["errorMessageColor"])

                # add this message to the list of errors
                self.__errors.append (message)
                
            print "%s:%s: %s" % (filename_str, line_str, msg)

        elif self.__file_msg_regex.match (message):
            # No line number, treat as an "introductory" line of text.
            self.__srcscan__(message, self.__colors["introColor"])
        else: # Anything else.       
            # Doesn't seem to be a warning or an error. Print normally.
            print self.__colors["reset"], message,

    def __srcscan__ (self, text, normalColor):
        #
        color_on  = self.__colors['srcColor']
        color_off = normalColor

        # This substitute replaces `foo' with `AfooB' where A is the escape
        # sequence that turns on the the desired source color, and B is the
        # escape sequence that returns to $normalColor.
        result = self.__src_regex.subn ("`" + color_on+ r'\1' + color_off + "'", text)

        return normalColor + result[0].strip() + self.__colors["reset"]


import subprocess
from ColorGcc import GccColorizer
import os, string

class BetterSpawn(object):
    def __init__(self, env):
        self._use_color = env["USE_COLOR"]
        self._gcc = GccColorizer()

    def __call__(self, shell, escape, executable, args, ENV):
        """
        A better alternative to scons spawn. We take control
        over subprocesses and if it is a g++ one, we reformat it's
        output
        """
        # Simply using args doesn't work. Who knows why. So we convert to a string
        # first.
        rargs = string.join(args)
        
        if self._use_color:
            redirect_stderr = None
            if executable.find("g++") != -1:
                redirect_stderr = self._gcc.colorize
            
            if redirect_stderr is None:
                proc = subprocess.Popen(args=rargs, env=ENV, shell=True)
            else:
                proc = subprocess.Popen(args=rargs, env=ENV, shell=True, stderr = subprocess.PIPE)

                redirect_stderr(proc.stderr)
        else:
            proc = subprocess.Popen(args=rargs, env=ENV, shell=True)

        return proc.wait()

def overwrite_spawn_function(env):
    env["SPAWN"] = BetterSpawn(env)


