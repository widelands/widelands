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
_re_blit = re.compile(r'(\d+),(\d+),(\d+),(\d+)@(-?\d+),(-?\d+)$')

class Animation(object):
    """
    Base class for various Animation representations.
    Note that all coordinates are in a numpy-friendly format,
    that is, rows come first.
    """
    def __init__(self):
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

    def get_cost(self, rectcost):
        raise NotImplemented()

class AnimationFullFrames(Animation):
    """
    Animation represented by full pictures for each frame
    """
    def __init__(self, frames=None):
        super(AnimationFullFrames, self).__init__()
        self.frames = frames or []
        if self.frames:
            self.shape = self.frames[0].pic.shape[:2]
            self.hotspot = (0,0)
            self.has_player_color = False if self.frames[0].pc_pic is None else True

    def get_nrframes(self):
        return len(self.frames)

    def get_frame(self, nr):
        return self.frames[nr]

    def get_cost(self, rectcost):
        return len(self.frames) * (rectcost + self.shape[0] * self.shape[1])


class Chunk(object):
    def __init__(self, pic, pc_pic=None):
        self.pic = pic
        self.pc_pic = pc_pic
        self.spritemap_ofs = None

Blit = collections.namedtuple('Blit', ('chunk', 'offset'))

class ChunkSet(object):
    def __init__(self, has_player_color):
        self.has_player_color = has_player_color
        self.chunks = []
        self.packing_shape = None
        self.spritemap_name = None

    def make_chunk(self, base_pic, base_pc_pic, rect):
        pic = base_pic[rect[0]:rect[2], rect[1]:rect[3]]
        if (base_pc_pic is not None) != self.has_player_color:
            raise Exception('inconsistent whether chunks have player color or not')
        if base_pc_pic is not None:
            pc_pic = base_pc_pic[rect[0]:rect[2], rect[1]:rect[3]]
        else:
            pc_pic = None
        for chunk in self.chunks:
            if chunk.pic.shape != pic.shape:
                continue
            if not np.all(chunk.pic == pic):
                continue
            if pc_pic is not None and not np.all(chunk.pc_pic == pc_pic):
                continue
            return chunk
        chunk = Chunk(pic, pc_pic)
        self.chunks.append(chunk)
        self.packing_shape = None
        self.spritemap_name = None
        return chunk

    def assign_packing(self, h, w, offsets):
        self.packing_shape = (h, w)
        for chunk, offset in zip(self.chunks, offsets):
            chunk.spritemap_ofs = offset

    def get_cost(self, rectcost):
        return sum([rectcost + chunk.pic.shape[0] * chunk.pic.shape[1] for chunk in self.chunks])

    def write_images(self, directory, spritemap_name):
        pic = np.zeros(self.packing_shape + (4,), np.uint8)
        if self.has_player_color:
            pc_pic = np.zeros(self.packing_shape + (4,), np.uint8)
        for chunk in self.chunks:
            pic[chunk.spritemap_ofs[0]:chunk.spritemap_ofs[0] + chunk.pic.shape[0],
                chunk.spritemap_ofs[1]:chunk.spritemap_ofs[1] + chunk.pic.shape[1]] = chunk.pic
            if self.has_player_color:
                pc_pic[chunk.spritemap_ofs[0]:chunk.spritemap_ofs[0] + chunk.pic.shape[0],
                       chunk.spritemap_ofs[1]:chunk.spritemap_ofs[1] + chunk.pic.shape[1]] = chunk.pc_pic

        self.spritemap_name = spritemap_name
        Image.fromarray(pic).save(directory + spritemap_name + '.png')
        if self.has_player_color:
            Image.fromarray(pc_pic).save(directory + spritemap_name + '_pc.png')


class AnimationBlits(Animation):
    def __init__(self, chunkset):
        super(AnimationBlits, self).__init__()
        self.chunkset = chunkset
        self.frames = []
        self.has_player_color = chunkset.has_player_color
        self.shape = (0, 0)
        self.hotspot = (0, 0)
        self.bbox = (0, 0, 0, 0)

    def append_frame(self, blits):
        self.frames.append(blits)
        bbox = (
            min([blit.offset[0] for blit in blits]),
            min([blit.offset[1] for blit in blits]),
            max([blit.offset[0] + blit.chunk.pic.shape[0] for blit in blits]),
            max([blit.offset[1] + blit.chunk.pic.shape[1] for blit in blits]),
        )
        self.bbox = (
            min(self.bbox[0], bbox[0]),
            min(self.bbox[1], bbox[1]),
            max(self.bbox[2], bbox[2]),
            max(self.bbox[3], bbox[3]),
        )
        self.shape = (self.bbox[2] - self.bbox[0], self.bbox[3] - self.bbox[1])
        self.hotspot = (-self.bbox[0], -self.bbox[1])

    def get_nrframes(self):
        return len(self.frames)

    def get_frame(self, idx):
        frame = self.frames[idx]
        pic = np.zeros(self.shape + (4,))
        if self.has_player_color:
            pc_pic = np.zeros(self.shape + (4,))
        else:
            pc_pic = None

        for blit in frame:
            destrect = (
                blit.offset[0] + self.hotspot[0],
                blit.offset[1] + self.hotspot[1],
                blit.offset[0] + self.hotspot[0] + blit.chunk.pic.shape[0],
                blit.offset[1] + self.hotspot[1] + blit.chunk.pic.shape[1],
            )
            pic[destrect[0]:destrect[2], destrect[1]:destrect[3]] = blit.chunk.pic
            if pc_pic is not None:
                pc_pic[destrect[0]:destrect[2], destrect[1]:destrect[3]] = blit.chunk.pc_pic

        return FullFrame(pic, pc_pic)

    def get_cost(self, rectcost):
        chunks = []
        for frame in self.frames:
            for blit in frame:
                for chunk in chunks:
                    if chunk is blit.chunk:
                        break
                else:
                    chunks.append(blit.chunk)
        return sum([
            rectcost + chunk.pic.shape[0] * chunk.pic.shape[1]
            for chunk in chunks
        ])

    def write(self, directory, conf_section):
        for key, value in self.options.iteritems():
            conf_section.set(key, value)
        conf_section.set('format', 'blits')
        conf_section.set('nrframes', str(len(self.frames)))
        conf_section.set('spritemap', self.chunkset.spritemap_name)
        for idx, blits in enumerate(self.frames):
            conf_section.set(
                '%d' % (idx),
                ';'.join([
                    '%d,%d,%d,%d@%d,%d' % (
                        blit.chunk.spritemap_ofs[1], blit.chunk.spritemap_ofs[0],
                        blit.chunk.pic.shape[1], blit.chunk.pic.shape[0],
                        blit.offset[1], blit.offset[0]
                    )
                    for blit in self.frames[idx]
                ])
            )

    @staticmethod
    def load(directory, section_dict):
        spritemap_name = section_dict.pop('spritemap')
        nrframes = int(section_dict.pop('nrframes'))

        spritemap = np.asarray(Image.open(directory + '/' + spritemap_name + '.png'))
        fn = directory + '/' + spritemap_name + '_pc.png'
        has_player_color = os.path.exists(fn)
        if has_player_color:
            spritemap_pc = np.asarray(Image.open(fn))
        else:
            spritemap_pc = None

        chunkset = ChunkSet(has_player_color)
        anim = AnimationBlits(chunkset)
        for framenr in xrange(nrframes):
            blitdescrs = [
                [int(s) for s in _re_blit.match(blit).groups()]
                for blit in section_dict.pop(str(framenr)).split(';')
            ]
            blits = []
            for blitdescr in blitdescrs:
                chunk = chunkset.make_chunk(
                    spritemap, spritemap_pc,
                    (blitdescr[1], blitdescr[0], blitdescr[1] + blitdescr[3], blitdescr[0] + blitdescr[2])
                )
                blits.append(Blit(chunk, (blitdescr[5], blitdescr[4])))
            anim.append_frame(blits)

        return anim


def load_glob(filename_glob):
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

    return AnimationFullFrames(rv)


def load_section(directory, section):
    d = dict([(key, value) for key, value in section.iterentries()])
    format = d.pop('format', None)
    typ = 2 if format == 'blits' else 1 if d.pop('packed', 'false').lower() == 'true' else 0
    if typ == 0:
        pics = d.pop('pics')
        anim = load_glob(directory + '/' + pics)
        anim.hotspot = tuple([int(v) for v in _re_point.match(d.pop('hotspot')).groups()[::-1]])
    elif typ == 2:
        anim = AnimationBlits.load(directory, d)
    else:
        raise Exception('cannot load this type of animation yet')
    anim.options.update(d)
    return anim

def load_conf(directory, anim):
    with open(directory + '/conf', 'r') as filp:
        conf = config.read(filp)
    section = conf.get_section(anim)
    return load_section(directory, section)
