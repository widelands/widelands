"""
Helper functions for reading and writing animations in
the various formats understood by Widelands
"""

from glob import glob
import collections
import Image
import numpy as np
import os
import re

import config

FullFrame = collections.namedtuple('FullFrame', ('pic', 'pc_pic'))

_re_point = re.compile('(\\d+)\\s+(\\d+)$')

class Group(object):
    pass


class Animation(object):
    """
    Base class for various Animation representations.
    Note that all coordinates are in a numpy-friendly format,
    that is, rows come first.
    """
    def __init__(self, group=None):
        self.group = group or Group()
        self.options = {}
        self.shape = None
        self.hotspot = None
        self.has_player_color = False

    def get_nrframes(self):
        """
        Return the number of frames in this animation
        """
        raise NotImplemented()

    def get_frame(self, nr):
        """
        Get a FullFrame object for the given frame number
        """
        raise NotImplemented()


class AnimationFullFrames(Animation):
    """
    Animation represented by full pictures for each frame
    """
    def __init__(self, frames=None, group=None):
        super(AnimationFullFrames, self).__init__(group)
        self.frames = frames or []
        if self.frames:
            self.shape = self.frames[0].pic.shape[:2]
            self.hotspot = (0,0)
            self.has_player_color = False if self.frames[0].pc_pic is None else True

    def get_nrframes(self):
        return len(self.frames)

    def get_frame(self, nr):
        return self.frames[nr]


def load_glob(filename_glob, group=None):
    """
    Load an animation from a list of image files matching
    the given glob pattern.
    """
    rv = []

    def _load(fn, seen_shape):
        img = np.asarray(Image.open(fn))
        if seen_shape and seen_shape != img.shape:
            raise Exception('Frame %s has different dimensions from previous frames' % (fn))
        return img

    seen_shape = None
    have_pc = None
    for idx, fn in enumerate(sorted(glob(filename_glob))):
        pic = _load(fn, seen_shape)
        if seen_shape is None:
            seen_shape = pic.shape
        pc_fn = os.path.splitext(fn)[0] + "_pc.png"
        pc_exists = os.path.exists(pc_fn)
        if have_pc is None:
            have_pc = pc_exists
        elif have_pc != pc_exists:
            raise Exception('Either all frames or none of the frames must have a pc image')
        if pc_exists:
            pc_pic = _load(pc_fn, seen_shape)
        else:
            pc_pic = None

        rv.append(FullFrame(pic, pc_pic))

    return AnimationFullFrames(rv, group)


def load_conf(directory, anim, subanim=None, group=None):
    with open(directory + '/conf', 'r') as filp:
        conf = config.read(filp)
    section = conf.get_section(anim)
    d = dict([(key, value) for key, value in section.iterentries()])
    typ = 1 if d.pop('packed', 'false').lower() == 'true' else 0
    if typ == 0:
        pics = d.pop('pics')
        anim = load_glob(directory + '/' + pics, group)
        anim.hotspot = tuple([int(v) for v in _re_point.match(d.pop('hotspot')).groups()[::-1]])
        anim.options.update(d)
    else:
        raise Exception('cannot load this type of animation yet')
    return anim
