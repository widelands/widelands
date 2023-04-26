"""Helper functions for reading and writing animations in the various formats
understood by Widelands."""

from glob import glob
import collections
import os
import re

from PIL import Image
import numpy as np

import pywi.config

FullFrame = collections.namedtuple('FullFrame', ('pic', 'pc_pic'))

_re_point = re.compile('(\\d+)\\s+(\\d+)$')
_re_blit = re.compile(r'(\d+),(\d+),(\d+),(\d+)@(-?\d+),(-?\d+)$')


class Context(object):
    def __init__(self):
        self.filenames = set()
        self.spritemap_names = set()


class Animation(object):
    """Base class for various Animation representations.

    Note that all coordinates are in a numpy-friendly format, that is,
    rows come first.
    """

    def __init__(self):
        self.options = {}
        self.shape = None
        self.hotspot = None
        self.has_player_color = False

    def get_nrframes(self):
        """Return the number of frames in this animation."""
        raise NotImplemented()

    def get_frame(self, nr):
        """Get a FullFrame object for the given frame number."""
        raise NotImplemented()

    def get_cost(self, rectcost):
        raise NotImplemented()


class AnimationFullFrames(Animation):
    """Animation represented by full pictures for each frame."""

    def __init__(self, frames=None):
        super(AnimationFullFrames, self).__init__()
        self.frames = frames or []
        if self.frames:
            self.shape = self.frames[0].pic.shape[:2]
            self.hotspot = (0, 0)
            self.has_player_color = False if self.frames[
                0].pc_pic is None else True

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

    def __lt__(self, other):
        """Orders by area."""
        return (
            (self.pic.shape[0] * self.pic.shape[1], self.pic.shape[0], self.pic.shape[1]) <
            (other.pic.shape[0] * other.pic.shape[1], other.pic.shape[0], other.pic.shape[1]))


Blit = collections.namedtuple('Blit', ('chunk', 'offset'))


class ChunkSet(object):
    def __init__(self, has_player_color):
        self.has_player_color = has_player_color
        self._chunks = []
        self.packing_shape = None
        self.spritemap_name = None

    def make_chunk(self, base_pic, base_pc_pic, rect):
        pic = base_pic[rect[0]:rect[2], rect[1]:rect[3]]
        if (base_pc_pic is not None) != self.has_player_color:
            raise Exception(
                'inconsistent whether chunks have player color or not')
        if base_pc_pic is not None:
            pc_pic = base_pc_pic[rect[0]:rect[2], rect[1]:rect[3]]
        else:
            pc_pic = None
        for chunk in self._chunks:
            if chunk.pic.shape != pic.shape:
                continue
            if not np.all(chunk.pic == pic):
                continue
            if pc_pic is not None and not np.all(chunk.pc_pic == pc_pic):
                continue
            return chunk
        chunk = Chunk(pic, pc_pic)
        self._chunks.append(chunk)
        self._chunks.sort(reverse=True)
        self.packing_shape = None
        self.spritemap_name = None
        return chunk

    @property
    def chunks(self):
        return self._chunks

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
        Image.fromarray(pic).save(directory + '/' + spritemap_name + '.png')
        if self.has_player_color:
            Image.fromarray(pc_pic[:, :, 0]).save(
                directory + '/' + spritemap_name + '_pc.png')


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
            pic[destrect[0]:destrect[2], destrect[
                1]:destrect[3]] = blit.chunk.pic
            if pc_pic is not None:
                pc_pic[destrect[0]:destrect[2], destrect[
                    1]:destrect[3]] = blit.chunk.pc_pic

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
                        blit.chunk.spritemap_ofs[
                            1], blit.chunk.spritemap_ofs[0],
                        blit.chunk.pic.shape[1], blit.chunk.pic.shape[0],
                        blit.offset[1], blit.offset[0]
                    )
                    for blit in self.frames[idx]
                ])
            )

    def copy(self, chunkset):
        """Create a copy of this animation that stores its chunk in the given
        chunkset."""
        if chunkset.has_player_color != self.has_player_color:
            raise Exception(
                'AnimationBlits.copy: inconsistent has_player_color')
        new_anim = AnimationBlits(chunkset)
        new_anim.options.update(self.options)
        for frame in self.frames:
            blits = []
            for blit in frame:
                chunk = chunkset.make_chunk(
                    blit.chunk.pic, blit.chunk.pc_pic,
                    (0, 0) + blit.chunk.pic.shape[0:2]
                )
                blits.append(Blit(chunk, blit.offset))
            new_anim.append_frame(blits)
        return new_anim

    @staticmethod
    def load(directory, section_dict, context=None):
        spritemap_name = section_dict.pop('spritemap')
        nrframes = int(section_dict.pop('nrframes'))

        fn = directory + '/' + spritemap_name + '.png'
        spritemap = np.asarray(Image.open(fn))
        if context is not None:
            context.filenames.add(fn)
            context.spritemap_names.add(spritemap_name)

        fn = directory + '/' + spritemap_name + '_pc.png'
        has_player_color = os.path.exists(fn)
        if has_player_color:
            spritemap_pc = np.asarray(Image.open(fn))
            if context is not None:
                context.filenames.add(fn)
            if len(spritemap_pc.shape) == 2:
                rgba = np.zeros(spritemap_pc.shape + (4,))
                rgba[:, :, 0] = spritemap_pc
                rgba[:, :, 1] = spritemap_pc
                rgba[:, :, 2] = spritemap_pc
                rgba[:, :, 3] = 255
                spritemap_pc = rgba
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
                    (blitdescr[1], blitdescr[0], blitdescr[1] +
                     blitdescr[3], blitdescr[0] + blitdescr[2])
                )
                blits.append(Blit(chunk, (blitdescr[5], blitdescr[4])))
            anim.append_frame(blits)

        return anim


def load_glob(filename_glob, context=None):
    """Load an animation from a list of image files matching the given glob
    pattern."""
    rv = []

    def _load(fn, seen_shape):
        if context is not None:
            context.filenames.add(fn)
        img = np.asarray(Image.open(fn).convert('RGBA'))
        assert len(img.shape) == 3

        if seen_shape and seen_shape != img.shape:
            raise Exception(
                'Frame %s has different dimensions from previous frames' % (fn))
        return img

    seen_shape = None
    have_pc = None
    for idx, fn in enumerate(sorted(glob(filename_glob))):
        pic = _load(fn, seen_shape)
        if seen_shape is None:
            seen_shape = pic.shape
        pc_fn = os.path.splitext(fn)[0] + '_pc.png'
        pc_exists = os.path.exists(pc_fn)
        if have_pc is None:
            have_pc = pc_exists
        elif have_pc != pc_exists:
            raise Exception(
                'Either all frames or none of the frames must have a pc image')
        if pc_exists:
            pc_pic = _load(pc_fn, seen_shape)
        else:
            pc_pic = None

        rv.append(FullFrame(pic, pc_pic))

    return AnimationFullFrames(rv)


def load_packed(directory, section_dict, context=None):
    pics = section_dict.pop('pics')
    hotspot = tuple(reversed([int(x)
                              for x in section_dict.pop('hotspot').split()]))
    dims = tuple(reversed([int(x)
                           for x in section_dict.pop('dimensions').split()]))
    base_offset = tuple(
        reversed([int(x) for x in section_dict.pop('base_offset').split()]))

    fn = directory + '/' + pics
    spritemap = np.asarray(Image.open(fn))
    if context is not None:
        context.filenames.add(fn)
        context.spritemap_names.add(pics.replace('.png', ''))

    fn = directory + '/' + pics.replace('.png', '_pc.png')
    has_player_color = os.path.exists(fn)
    if has_player_color:
        spritemap_pc = np.asarray(Image.open(fn))
        if context is not None:
            context.filenames.add(fn)
        if len(spritemap_pc.shape) == 2:
            rgba = np.zeros(spritemap_pc.shape + (4,))
            rgba[:, :, 0] = spritemap_pc
            rgba[:, :, 1] = spritemap_pc
            rgba[:, :, 2] = spritemap_pc
            rgba[:, :, 3] = 255
            spritemap_pc = rgba
    else:
        spritemap_pc = None

    nrframes = 1
    regions = []
    for name in section_dict.keys():
        if not name.startswith('region_'):
            continue
        region_descr = section_dict.pop(name).split(':')
        region = (tuple([int(x) for x in region_descr[0].split()]), [
            tuple([int(x) for x in frame.split()])
            for frame in region_descr[1].split(';')
        ])
        if not regions:
            nrframes = len(region[1])
        else:
            if nrframes != len(region[1]):
                raise Exception(
                    'inconsistent number of frames in packed=true animation')
        regions.append(region)

    chunkset = ChunkSet(has_player_color)
    anim = AnimationBlits(chunkset)

    base_chunk = chunkset.make_chunk(
        spritemap, spritemap_pc,
        (base_offset[0], base_offset[1], base_offset[
         0] + dims[0], base_offset[1] + dims[1])
    )
    for framenr in xrange(nrframes):
        blits = [Blit(base_chunk, (-hotspot[0], -hotspot[1]))]
        for region in regions:
            ofs = (region[1][framenr][1], region[1][framenr][0])
            chunk = chunkset.make_chunk(
                spritemap, spritemap_pc,
                (ofs[0], ofs[1], ofs[0] + region[0][3], ofs[1] + region[0][2])
            )
            blits.append(
                Blit(chunk, (region[0][1] - hotspot[0], region[0][0] - hotspot[1])))
        anim.append_frame(blits)

    return anim


def load_section(directory, section, context=None):
    d = dict([(key, value) for key, value in section.iterentries()])
    format = d.pop('format', None)
    typ = 2 if format == 'blits' else 1 if d.pop(
        'packed', 'false').lower() == 'true' else 0
    if typ == 0:
        pics = d.pop('pics')
        anim = load_glob(directory + '/' + pics, context)
        anim.hotspot = tuple(
            [int(v) for v in _re_point.match(d.pop('hotspot')).groups()[::-1]])
    elif typ == 2:
        anim = AnimationBlits.load(directory, d, context)
    elif typ == 1:
        anim = load_packed(directory, d, context)
    else:
        raise Exception('cannot load this type of animation yet')
    anim.options.update(d)
    return anim


def load_legacy_diranims(directory, name, section, context=None):
    d = dict([(key, value) for key, value in section.iterentries()])
    dirpics = d.pop('dirpics')
    hotspot = tuple([int(v)
                     for v in _re_point.match(d.pop('hotspot')).groups()[::-1]])
    animations = {}
    for direction in ['e', 'ne', 'nw', 'w', 'sw', 'se']:
        anim = load_glob(directory + '/' +
                         dirpics.replace('!!', direction), context)
        anim.hotspot = hotspot
        anim.options.update(d)
        animations['%s_%s' % (name, direction)] = anim
    return animations


def load_conf(directory, anim, context=None):
    with open(directory + '/conf', 'r') as filp:
        conf = pywi.config.read(filp)
    section = conf.get_section(anim)
    return load_section(directory, section, context)
