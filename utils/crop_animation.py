#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2010 by the Widelands Development Team
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
#
# This script takes one or more conf files on the command line.
# It reads [idle] and [working] sections from these files and
# take the pics= and playercolor= lines from these sections.
#
#
#
#

import os
import re
import sys

from PIL import Image

"""
This script takes a list of conf files as arguments and crops
the correponding animations to their proper size

usage:
    find -type f -name conf -exec crop_animation.py {} \;
"""

def crop_images(basepath, section):
    """
    crop_images does the work of actually cropping the animation pictures
    from a conf section.  The first parameter basepath is the directory
    in which the conf file was.  The second parameter section is a
    dictionary with contains the important contents of the section from
    the conf file
    """
    # two empty lists. The first for the animation pictures
    list=[]
    # the second one for the player color masks
    list_pc=[]

    basepath=basepath + "/"

    if 'pics' in section:
        filepattern=section['pics']
    else:
        filepattern=section['dirpics']

    # TODO(sirver): playercolor is never used in the conf files anymore. remove this.
    use_playercolor=False
    if 'playercolor' in section and \
       section['playercolor'].upper().strip()=="TRUE":
        use_playercolor=True

    # replace the placeholders ?? and !! by a .
    filepattern=filepattern.replace(r".",r"\.")
    filepattern=filepattern.replace(r"??",r"[0-9][0-9]")
    filepattern=filepattern.replace(r"!!",r"(e|w|sw|se|nw|ne)")
    # The end of the pattern is the end of the filename
    filepattern+="$"

    # create a pattern for the playercolor masks
    filepattern_pc=filepattern.rpartition("\.")
    filepattern_pc=filepattern_pc[0]+"_pc\\."+filepattern_pc[2]

    # print filepattern
    # print filepattern_pc

    for filename in os.listdir(basepath):
        if re.match(filepattern, filename):
            list.append(filename)
        if use_playercolor and re.match(filepattern_pc, filename):
            list_pc.append(filename)

    # Now we have two list which contains the animation pictures and the
    # playercolor mask

    #print list
    #print list_pc

    if len(list)<1:
        return "error" # we found no pictures

    # Now open the first picture, get the size and validate the size
    im=Image.open(basepath+list[0])
    size=im.size;
    if size[0]<1 or size[1]<1:
        print "*** ERROR ***: image has size 0"
        return "error"
    # Set a first boundingbox to crop away the maximum from the picture
    crop=[size[0], size[1], 0, 0]

    # Now go through all picture of the animation and get the boundingbox
    for file in list:
        #print basepath+file
        im = Image.open(basepath+file)
        #im.show()
        bb = im.getbbox();
        #print "Bounding box for " + file + ": ",
        #print bb

        # Here the boundingbox is adjusted. It's only set it to crop away less
        # of the picture
        if bb[0]<crop[0]:
            crop[0]=bb[0]
        if bb[1]<crop[1]:
            crop[1]=bb[1]
        if bb[2]>crop[2]:
            crop[2]=bb[2]
        if bb[3]>crop[3]:
            crop[3]=bb[3]
        # Assure that the size of all images of the animation is the same
        if im.size!=size:
            print "*** Error *** Frame size changed", basepath, " ",file
            return "error"

    # Now we have a common boundingbox of all images of the animation

    #print crop,
    #print " ",
    #print size,
    if crop[0]>0 or crop[1]>0 or crop[2]<size[0] or crop[3]<size[1]:
        # We have somethin to crop
        rect=(crop[0],crop[1],crop[2],crop[3])
        print "crop images", filepattern, filepattern_pc

        # If playercolor was true in the config file add the playercolor masks
        # tho the list
        if use_playercolor:
            list += list_pc

        #print list

        # Now go through all images (include playercolor mask if set) and crop
        # them
        for file in list:
            im=Image.open(basepath+file)
            #print "crop", basepath+file, " ",im.size, rect
            im=im.crop(rect)
            # Images in paletted mode have to be handled differently. If they
            # are saved without the option the transparency is lost
            if(im.mode=="P"):
                im.save(basepath+file, quality=100, transparency=0)
            else:
                im.save(basepath+file, quality=100)
            # If this function cropped something it returns a tupple containing
            # the crop rect
        return rect
    else:
        #print "nothing to do for", filepattern, filepattern_pc
        return "unchanged"


def read_conffile(filename):
    """
    read_conf reads a configfile and searches for sections which contain
    pic= or dirpic=. Such section are added to a list. For every of
    these sections the function crop_images() is called. of crop_images
    did not return "error" or "unchanged" the hotspot in the conf file
    is adjusted by the top and left values of the crop rectangle.  The
    parameter to this function is the conf file
    """
    print "read " +  filename

    # We read the complete file to a list of lines
    f = open(filename)
    lines = f.readlines()
    f.close()

    basepath=os.path.split(filename)
    basepath=basepath[0]
    #print basepath

    # section_begin track where the current section began
    section_begin=0
    # insection a list of matching section is saved
    sections=[]
    # i counts the lines
    i=0
    # section is a dictionary where the contents of the current section are
    # stored
    section={}

    # First go through all lines and identify relevant sections
    for line in lines:
        line = line.split("#")
        line = line[0].strip(" \r\n\t")
        if re.match(r"\[[a-zA-Z0-9]*\]" , line):
            #print "*** section begin"
            if "pics" in section or "dirpics" in section:
                sections.append((section_begin,i-1,section))
            section_begin=i
            section={'name':'test'}
        if re.match(r"(pics=|dirpics=|hotspot=|playercolor=)", line):
            keyval=line.split("=")
            section[keyval[0]]=keyval[1]
        i+=1
    if "pics" in section or "dirpics" in section:
        pic_section=False
        sections.append((section_begin,i-1,section))

    # Now use the information gatherd above to crop the images
    for section in sections:
        #print section
        # Crop the animation ...
        ret = crop_images(basepath, section[2])

        # See if we cropped images
        if ret!="error" and ret!="unchanged":
            if (ret[0]>0 or ret[1]>0) and 'hotspot' in section[2]:
                # Somethin was cropped from top or left, so adjust the hotspot
                print "change hotspot", ret
                oldhs=section[2]['hotspot'].split(" ")
                newhs=(int(oldhs[0])-ret[0],int(oldhs[1])-ret[1])
                #print oldhs, newhs
                # search all section which use this animation
                for sec in sections:
                    if "pics" in section[2]:
                        fname=section[2]['pics']
                    else:
                        fname=section[2]['dirpics']
                    if ('pics' in sec[2] and sec[2]['pics']==fname) or \
                       ('dirpics' in sec[2] and sec[2]['dirpics']==fname):
                        # Simply go through the lines of the section and
                        # search for hotspot= if found replace the line. It
                        # will be written to the conf file a the end
                        for i in range(sec[0], sec[1]+1):
                            #print i,
                            #print lines[i],
                            if re.match(r"hotspot=", lines[i]):
                                lines[i]="hotspot=%u %u\n" % (newhs[0], newhs[1])
                                #print i, lines[i],

    # Now we write back the conf file from our list of lines
    f = open(filename,"w")
    f.writelines(lines)
    f.close()


def main():
    """
    This is the main function. It calls read_conffile() for every file
    given to this program
    """
    if len(sys.argv)<=1:
        print "Usage: " + sys.argv[0] + " conf-files"
        sys.exit()

    for file in sys.argv[1:]:
        if os.path.isfile(file):
            read_conffile(file)
        else:
            print "*** WARNING ***: " + file + " is not a file"

if __name__ == '__main__':
    main()
