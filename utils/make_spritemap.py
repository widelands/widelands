#!/usr/bin/env python
# encoding: utf-8

from glob import glob
from itertools import chain, combinations, permutations
import collections
import md5
import re
import argparse
import math
import os
import sys

import Image
import numpy as np
from scipy import ndimage

import pywi.animation
import pywi.config
import pywi.packing

OriginalFrame = collections.namedtuple('OriginalFrame', ('pic', 'pc_pic', 'anim', 'idx'))

# Set partition: http://code.activestate.com/recipes/576795/
def set_partition(iterable, chain=chain, map=map):
    """Returns all set partitions of iteratable"""
    s = iterable if hasattr(iterable, '__getslice__') else tuple(iterable)
    n = len(s)
    first, middle, last = [0], range(1, n), [n]
    getslice = s.__getslice__
    return list(map(getslice, chain(first, div), chain(div, last)) for i in
            range(n) for div in combinations(middle, i))

def set_partitions_all_permutation(iterable):
    """Returns all unique set partitions in all permutations of iteratable"""
    def _sub():
        for partition in set_partition(iterable):
            for idx in range(len(partition)):
                for c in permutations(partition[idx], len(partition[idx])):
                    yield tuple(map(tuple, partition[:idx] + [list(c)] + partition[idx+1:]))
    return sorted(set(_sub()))


class Point(object):
    def __init__(self, y, x):
        self.y = y
        self.x = x

class Rect(object):
    def __init__(self, y1, y2, x1, x2):
        self.top = y1
        self.left = x1
        self.bottom = y2
        self.right = x2

    @property
    def w(self):
        return self.right - self.left
    @property
    def h(self):
        return self.bottom - self.top

    @property
    def tl(self):
        "top-left point"
        return Point(self.top, self.left)
    @property
    def tr(self):
        return Point(self.top, self.right)
    @property
    def bl(self):
        return Point(self.bottom, self.left)
    @property
    def br(self):
        return Point(self.bottom, self.right)

    def overlapping(self, r):
        return any(r.contains(p) for p in (self.tr, self.br, self.bl, self.tl)) or \
                any(self.contains(p) for p in (r.tr, r.br, r.bl, r.tl))

    def merge(self, r):
        """This yields a new rect that just contains both regions."""
        return Rect(
            min(self.top,r.top),
            max(self.bottom,r.bottom),
            min(self.left,r.left),
            max(self.right,r.right)
        )

    def contains(self, p):
        # Note: borders are inclusive for this check
        if self.top <= p.y <= self.bottom:
            if self.left <= p.x <= self.right:
                return True
        return False

class ImageWrapper(object):
    def __init__(self, img, pc_img, r, id):
        self.img = img
        self.pc_img = pc_img
        self.id = tuple(id)
        self.node = None

    @property
    def w(self):
        return self.img.shape[1]

    @property
    def h(self):
        return self.img.shape[0]

    def __lt__(self, o):
        if max(self.w,self.h) < max(o.w,o.h): return True
        if max(self.w,self.h) > max(o.w,o.h): return False
        if self.w < o.w: return True
        if self.w > o.w: return False
        if self.h < o.h: return True
        if self.h > o.h: return False
        return self.id < o.id



def merge_overlapping(regions):
    for i in range(len(regions)):
        for j in range(len(regions)):
            if i == j: continue
            if regions[i].overlapping(regions[j]):
                r1, r2 = regions[i], regions[j]
                regions.remove(r1)
                regions.remove(r2)
                regions.append(r1.merge(r2))
                return True
    return False

def cut_out_main(img, regions):
    img = img.copy()
    for r in regions:
        img[r.top:r.bottom+1, r.left:r.right+1] = (0,0,0,0)
    return img

def eliminate_duplicates(imgs):
    for i in range(len(imgs)):
        for j in range(i+1, len(imgs)):
            if (imgs[i][0] == imgs[j][0]).all():
                imgs[i] = (imgs[i][0], imgs[i][1], imgs[i][2] + imgs[j][2])
                del imgs[j]
                return True
    return False


def prepare_animations(anims):
    imgs, pc_imgs = load_animations(anims)

    base_pic = imgs[0][0]
    pc_base_pic = pc_imgs[0] if pc_imgs else None
    pics_to_fit = []

    # Find the regions that differ over all frames
    diff = np.zeros(base_pic.shape, np.uint32)
    for i in range(len(imgs)):
        for j in range(i+1, len(imgs)):
            diff += abs(imgs[i][0] - imgs[j][0])

    label_img, nlabels = ndimage.label(diff.max(axis=-1) > 0)
    regions = []
    for i in range(1, nlabels + 1):
        ys, xs = np.where(label_img==i)
        # Note, this rectangular regions are including the end points.
        regions.append(Rect(ys.min(),ys.max(),xs.min(),xs.max()))

    while merge_overlapping(regions):
        pass

    pics_to_fit.append(ImageWrapper(
        cut_out_main(base_pic, regions),
        cut_out_main(pc_base_pic, regions) if pc_base_pic is not None else None,
        Rect(0, base_pic.shape[0], 0, base_pic.shape[1]),
        [(anim, -1, -1) for anim in anims])
    )
    for ridx,r in enumerate(regions):
        reg_imgs = []
        for idx in range(len(imgs)):
            img,anim,framenr = imgs[idx]
            subimg = img[r.top:r.bottom+1, r.left:r.right+1]
            reg_imgs.append((subimg, pc_imgs[idx] if pc_imgs else None, [(anim, ridx, framenr)]))

        # Drop the exact same images in this region.
        while eliminate_duplicates(reg_imgs):
            pass

        for subimg, pc_img, id in reg_imgs:
            pc_subimg = None
            if pc_img is not None:
                pc_subimg = pc_img[r.top:r.bottom+1, r.left:r.right+1]
            pics_to_fit.append(ImageWrapper(subimg, pc_subimg, r, id))
    return pics_to_fit, regions, base_pic.shape[1], base_pic.shape[0]

##############################################
##############################################
##############################################

# Consider an additional fragment/rectangle to be beneficial if it saves this many pixels in image data
FRAGMENT_COST = 32


def macr_exact_bruteforce(bitmask, lower_range=None, upper_range=None, FRAGMENT_COST=FRAGMENT_COST):
    """
    Compute a Minimum Average Cost Rectangle among all rectangles with lower left corner in lower_range
    and upper right corner in upper_range, by computing the cost of all possible rectangles.

    Returns (cost, rectangle)

    Note: Returns a rectangle with cost strictly greater than FRAGMENT_COST + 1 if bitmask contains
    no set pixels.
    """
    if bitmask.shape[0] * bitmask.shape[1] > 4000:
        raise Exception('macr_exact_bruteforce called on a large bitmask')

    lower_range = lower_range or ((0, 0), bitmask.shape)
    upper_range = upper_range or ((0, 0), bitmask.shape)
    lower_ext = tuple(np.subtract(lower_range[1], lower_range[0]))
    upper_ext = tuple(np.subtract(upper_range[1], upper_range[0]))
    cum0 = bitmask.cumsum(0)
    cum1 = bitmask.cumsum(1)
    cum01 = cum0.cumsum(1)

    tr = cum01[upper_range[0][0]:upper_range[1][0], upper_range[0][1]:upper_range[1][1]]
    tr = np.tile(tr.reshape((1,1) + upper_ext), lower_ext + (1,1))
    tl = (cum01 - cum0)[upper_range[0][0]:upper_range[1][0], lower_range[0][1]:lower_range[1][1]]
    tl = np.tile(tl.transpose().reshape((1, lower_ext[1], upper_ext[0], 1)), (lower_ext[0], 1, 1, upper_ext[1]))
    br = (cum01 - cum1)[lower_range[0][0]:lower_range[1][0], upper_range[0][1]:upper_range[1][1]]
    br = np.tile(br.reshape((lower_ext[0], 1, 1, upper_ext[1])), (1, lower_ext[1], upper_ext[0], 1))
    bl = (cum01 - cum0 - cum1 + bitmask)[lower_range[0][0]:lower_range[1][0], lower_range[0][1]:lower_range[1][1]]
    bl = np.tile(bl.reshape(lower_ext + (1,1)), (1,1) + upper_ext)

    indices = np.indices(lower_ext + upper_ext)
    covered = (tr - tl - br + bl) * ((indices[2] >= indices[0]) & (indices[3] >= indices[1]))

    cost = (
        ((upper_range[0][0] - lower_range[0][0]) + indices[2] - indices[0] + 1) *
        ((upper_range[0][1] - lower_range[0][1]) + indices[3] - indices[1] + 1)
    )
    cost = FRAGMENT_COST + np.fmax(cost, 1)
    #print cost

    avg_cost = cost / np.fmax(covered, 0.1)
    #print avg_cost
    argmin_local = np.unravel_index(np.argmin(avg_cost), avg_cost.shape)
    argmin = (
        argmin_local[0] + lower_range[0][0],
        argmin_local[1] + lower_range[0][1],
        argmin_local[2] + upper_range[0][0] + 1,
        argmin_local[3] + upper_range[0][1] + 1
    )
    return avg_cost[argmin_local], argmin

def minimum_average_cost_rectangle(bitmask, FRAGMENT_COST=FRAGMENT_COST):
    """
    Compute a rectangle that minimizes (FRAGMENT_COST + Area) / (Pixels Covered in bitmask)

    Returns (cost, rectangle)
    """
    return macr_exact_bruteforce(bitmask, FRAGMENT_COST=FRAGMENT_COST)

    # Setup tiling hierarchy
    MAX_TILES = 16
    AVG_COST_INF = FRAGMENT_COST + 2

    tileshapes = []
    remaindershape = bitmask.shape
    while remaindershape[0] > MAX_TILES or remaindershape[1] > MAX_TILES:
        tiles = tuple([
            int(math.ceil(rs ** (1.0 / int(math.ceil(math.log(rs, MAX_TILES))))))
            for rs in remaindershape
        ])
        tileshapes.append(tiles)
        remaindershape = tuple([
            (rs + ts - 1) / ts
            for ts, rs in zip(tiles, remaindershape)
        ])
    tileshapes.append(remaindershape)
    shape = tuple(np.prod(tileshapes, 0))
    print 'original:', bitmask.shape, 'shapes:', tileshapes, 'total shape:', shape
    if shape != bitmask.shape:
        extend = np.zeros(shape)
        extend[:bitmask.shape[0], :bitmask.shape[1]] = bitmask
        bitmask = extend

def minimum_average_cost_grow(bitmask, rectangle):
    """
    Find the best way to grow the given rectangle in one direction only,
    in terms of (Increased Area) / (Additionally Covered Pixels in bitmask).

    Returns (cost, rectangle), where cost is None if no additional pixels can be covered.
    """
    best_cost = None
    best_rectangle = rectangle
    height = rectangle[2] - rectangle[0]
    width = rectangle[3] - rectangle[1]

    # grow positive in axis 1
    additional = bitmask[rectangle[0]:rectangle[2], rectangle[3]:].sum(0).cumsum()
    if additional.shape[0]:
        cost = np.arange(height, (additional.shape[0] + 1) * height, height, np.float)
        inv = additional / cost # avoid division by zero
        best = inv.argmax()
        if inv[best] > 0:
            best_cost = 1.0 / inv[best]
            best_rectangle = (rectangle[0], rectangle[1], rectangle[2], rectangle[3] + 1 + best)

    # grow negative in axis 1
    additional = bitmask[rectangle[0]:rectangle[2], :rectangle[1]].sum(0)[::-1].cumsum()
    if additional.shape[0]:
        cost = np.arange(height, (additional.shape[0] + 1) * height, height, np.float)
        inv = additional / cost # avoid division by zero
        best = inv.argmax()
        if inv[best] > 0 and (best_cost is None or best_cost > 1.0 / inv[best]):
            best_cost = 1.0 / inv[best]
            best_rectangle = (rectangle[0], rectangle[1] - 1 - best, rectangle[2], rectangle[3])

    # grow positive in axis 0
    additional = bitmask[rectangle[2]:, rectangle[1]:rectangle[3]].sum(1).cumsum()
    if additional.shape[0]:
        cost = np.arange(width, (additional.shape[0] + 1) * width, width, np.float)
        inv = additional / cost # avoid division by zero
        best = inv.argmax()
        if inv[best] > 0 and (best_cost is None or best_cost > 1.0 / inv[best]):
            best_cost = 1.0 / inv[best]
            best_rectangle = (rectangle[0], rectangle[1], rectangle[2] + 1 + best, rectangle[3])

    # grow negative in axis 1
    additional = bitmask[:rectangle[0], rectangle[1]:rectangle[3]].sum(1)[::-1].cumsum()
    if additional.shape[0]:
        cost = np.arange(width, (additional.shape[0] + 1) * width, width, np.float)
        inv = additional / cost # avoid division by zero
        best = inv.argmax()
        if inv[best] > 0 and (best_cost is None or best_cost > 1.0 / inv[best]):
            best_cost = 1.0 / inv[best]
            best_rectangle = (rectangle[0] - 1 - best, rectangle[1], rectangle[2], rectangle[3])

    return (best_cost, best_rectangle)

def draw_bitmask(bitmask):
    for y in range(bitmask.shape[0]):
        print ''.join([(' ', '*')[cell] for cell in bitmask[y]])

def bitmask_from_rectangle(shape, rectangle):
    idx = np.indices(shape)
    return np.logical_and(
        np.logical_and(idx[0] >= rectangle[0], idx[0] < rectangle[2]),
        np.logical_and(idx[1] >= rectangle[1], idx[1] < rectangle[3])
    )

def draw_rectangles_over_bitmask(bitmask, rectangles):
    if rectangles:
        coverage = np.max(
            [np.where(bitmask_from_rectangle(bitmask.shape, rect), num+1, 0) for num, rect in enumerate(rectangles)],
            axis=0
        )
    else:
        coverage = np.zeros(bitmask.shape)
    for bmrow, crow in zip(bitmask, coverage):
        print ''.join([str(c) if c > 0 else '*' if b else ' ' for b, c in zip(bmrow, crow)])

def draw_frame_diffs(frames):
    npframes = np.array([frame.pic for frame in frames])
    any_not_transparent = np.any(npframes[:,:,:,3] != 0, axis=0)
    all_equal_not_transparent = np.logical_and(
        npframes[0,:,:,3] != 0,
        np.all(np.all(npframes[0:1,:,:,:] == npframes, axis=-1), axis=0)
    )
    for eqrow, ntrow in zip(all_equal_not_transparent, any_not_transparent):
        print ''.join(['.' if eq else '*' if nt else ' ' for eq, nt in zip(eqrow, ntrow)])

def rectangle_cost(rectangle, FRAGMENT_COST=FRAGMENT_COST):
    return FRAGMENT_COST + (rectangle[2] - rectangle[0]) * (rectangle[3] - rectangle[1])

def compute_rectangle_covering(bitmask, FRAGMENT_COST=FRAGMENT_COST):
    """
    Given a bitmask of pixels, find a list of rectangles that covers all pixels of the mask that are set to true,
    with a goal of minimizing Total Area of Covering Rectangles + FRAGMENT_COST * Number of Covering Rectangles.

    Returns (cost, list of rectangles)
    """
    # This implements the simple set cover heuristic,
    # i.e. it greedily covers pixels by adding a rectangle with minimum cost
    # per covered pixel, or by extending an existing rectangle.
    rectangles = []
    indices = np.indices(bitmask.shape)
    remainder = bitmask
    while np.count_nonzero(remainder) != 0:
        new_rectangle_score, new_rectangle = minimum_average_cost_rectangle(
            remainder,
            FRAGMENT_COST=FRAGMENT_COST
        )
        best_score = new_rectangle_score
        best_rectangles = rectangles + [new_rectangle]

        for idx, rect in enumerate(rectangles):
            grow_score, grow_rect = minimum_average_cost_grow(remainder, rect)
            if grow_score is not None and grow_score <= best_score:
                best_score = grow_score
                best_rectangles = rectangles[:idx] + rectangles[idx + 1:] + [grow_rect]

        #print 'best score', best_score
        #print 'rectangles:', best_rectangles

        rectangles = best_rectangles
        remainder = np.logical_and(
            remainder,
            np.logical_or(
                np.logical_or(indices[0] < rectangles[-1][0], indices[0] >= rectangles[-1][2]),
                np.logical_or(indices[1] < rectangles[-1][1], indices[1] >= rectangles[-1][3])
            )
        )
    return sum([rectangle_cost(rect, FRAGMENT_COST=FRAGMENT_COST) for rect in rectangles]), rectangles

def build_frame_group(frames, FRAGMENT_COST=FRAGMENT_COST):
    """
    Given a list of frames, find a shared base picture based on the first frame,
    as well as individual deltas.

    Returns (cost, base_pic, base_pic_rect, [frame_rectangles])
    """
    shape = frames[0].pic.shape
    all_opaque_mask = np.all([frame.pic[:,:,3] == 255 for frame in frames], 0)
    base_pic_mask = frames[0].pic[:,:,3] != 0
    for frame in frames[1:]:
        base_pic_mask &= np.all(frame.pic == frames[0].pic, -1) | all_opaque_mask
    p = np.argwhere(base_pic_mask)
    base_pic_min = np.min(p, 0)
    base_pic_max = np.max(p, 0)
    base_pic_rect = (base_pic_min[0], base_pic_min[1], base_pic_max[0] + 1, base_pic_max[1] + 1)
    base_pic_cost = rectangle_cost(base_pic_rect, FRAGMENT_COST=FRAGMENT_COST)
    base_pic = np.where(np.reshape(base_pic_mask, (shape[0], shape[1], 1)), frames[0].pic, 0)

    total_cost = base_pic_cost
    frame_rectangles = []
    for frame in frames:
        delta_mask = (frame.pic[:,:,3] != 0) & np.any(frame.pic != base_pic, -1)
        cost, rectangles = compute_rectangle_covering(delta_mask, FRAGMENT_COST=FRAGMENT_COST)
        total_cost += cost
        frame_rectangles.append(rectangles)

    return (total_cost, base_pic, base_pic_rect, [frame_rectangles])

def pack_frames_global_bbox(frames, FRAGMENT_COST=FRAGMENT_COST):
    """
    Pack frames by taking a global bounding box
    """
    npframes = np.array([frame.pic for frame in frames])
    any_transparent = np.any(npframes[:,:,:,3] != 0, axis=0)
    p = np.argwhere(any_transparent)
    pic_min = np.min(p, 0)
    pic_max = np.max(p, 0)
    bbox_rect = (pic_min[0], pic_min[1], pic_max[0] + 1, pic_max[1] + 1)
    total_cost = len(frames) * rectangle_cost(bbox_rect, FRAGMENT_COST=FRAGMENT_COST)
    return total_cost

def pack_frames_bbox(frames, FRAGMENT_COST=FRAGMENT_COST):
    """
    Pack frames by simply taking the bounding box of each frame
    """
    shape = frames[0].pic.shape
    total_cost = 0

    print 'pack_frames_bbox: packing', len(frames), 'frames'
    for idx, frame in enumerate(frames):
        print ' frame', idx,
        pic_mask = frame.pic[:,:,3] != 0
        p = np.argwhere(pic_mask)
        pic_min = np.min(p, 0)
        pic_max = np.max(p, 0)
        bbox_rect = (pic_min[0], pic_min[1], pic_max[0] + 1, pic_max[1] + 1)
        cost = rectangle_cost(bbox_rect, FRAGMENT_COST=FRAGMENT_COST)
        print 'cost', cost
        total_cost += cost

    return total_cost

def pack_frames_greedy(frames, FRAGMENT_COST=FRAGMENT_COST):
    """
    Find a packing of the frames into frame groups.

    Greedily adds frames to frame groups as long as the average cost per frame
    decreases. As a heuristic, frames with high pixel overlap are combined first.
    """
    MAXREJECT = 10
    shape = frames[0].pic.shape
    uncovered = [idx for idx in range(len(frames))]
    framegroups = []
    total_cost = 0

    print 'pack_frames_greedy: packing', len(frames), 'frames'
    while uncovered:
        leader = uncovered[0]
        del uncovered[0]
        print ' start new framegroup with leader', leader,
        framegroup = build_frame_group([frames[leader]], FRAGMENT_COST=FRAGMENT_COST)
        avgcost = framegroup[0]
        print 'cost', avgcost

        leader_pic = frames[leader].pic
        trials = zip(uncovered, [
            np.count_nonzero(np.all(leader_pic == frames[u].pic, -1))
            for u in uncovered
        ])
        trials.sort(key=lambda t: -t[1])

        followers = []
        rejections = 0
        for trial, nrcommonpix in trials:
            try_frames = [leader] + followers + [trial]
            print '  try adding %d with %d common pixels for %s...' % (
                trial, nrcommonpix, try_frames
            ),
            new_framegroup = build_frame_group(
                [frames[i] for i in try_frames],
                FRAGMENT_COST=FRAGMENT_COST
            )
            new_avgcost = float(new_framegroup[0]) / (len(followers) + 2)
            print 'avgcost', new_avgcost,
            if new_avgcost > avgcost:
                rejections += 1
                if rejections >= MAXREJECT:
                    print 'reject and stop'
                    break
                else:
                    print 'reject'
            else:
                print 'add'
                framegroup = new_framegroup
                avgcost = new_avgcost
                followers.append(trial)
                uncovered.remove(trial)
                rejections = 0

        print ' adding framegroup', [leader] + followers, 'cost', framegroup[0]
        framegroups.append(framegroup)
        total_cost += framegroup[0]

    return total_cost

##############################################
##############################################
##############################################

def output_results(anim, img_name, dimensions, regions, offsets_by_id, args):
    with open("conf", "a") as f:
        f.write("[%s]\n" % anim)
        f.write("packed=true\n")  # NOCOM(#sirver): get rid of this again.
        f.write("pics=%s\n" % img_name)
        f.write("dimensions=%i %i\n" % (dimensions[0], dimensions[1]))
        has_plr_clr = False
        if len(regions) and (anim + "_pc", 0, 0) in offsets_by_id:
            has_plr_clr = True

        def _find_offsets(anim, ridx):
            cur_fr = 0
            rv = []
            while True:
                id = (anim, ridx, cur_fr)
                if id not in offsets_by_id:
                    break
                rv.append("%i %i" % offsets_by_id[id])
                cur_fr += 1
            return ";".join(rv)

        f.write("base_offset=%i %i\n" % offsets_by_id[anim, -1, -1])

        for ridx,r in enumerate(regions):
            f.write("region_%02i=%i %i %i %i:%s\n" % (
                ridx, r.left, r.top, r.right - r.left + 1,
                r.bottom - r.top + 1, _find_offsets(anim, ridx)
                ))

        if args.fps:
            f.write("fps=%s\n" % args.fps)
        if args.hotspot:
            f.write("hotspot=%s\n" % args.hotspot)
        f.write("\n")

#############################################
#############################################
#############################################


def optimize_bbox(animations, chunksets):
    """
    Joint optimization of the given animations using a simple bounding box routine
    """
    print 'Running bbox optimization...'
    new_animations = {}
    for name, anim in animations.iteritems():
        print 'Optimizing %s' % (name)
        if chunksets[anim.has_player_color] is None:
            chunksets[anim.has_player_color] = pywi.animation.ChunkSet(anim.has_player_color)
        chunkset = chunksets[anim.has_player_color]
        new_anim = pywi.animation.AnimationBlits(chunkset)
        new_anim.options.update(anim.options)
        for idx in xrange(anim.get_nrframes()):
            frame = anim.get_frame(idx)
            pic_mask = frame.pic[:,:,3] != 0
            p = np.argwhere(pic_mask)
            pic_min = np.min(p, 0)
            pic_max = np.max(p, 0)
            bbox_rect = (pic_min[0], pic_min[1], pic_max[0] + 1, pic_max[1] + 1)
            chunk = chunkset.make_chunk(frame.pic, frame.pc_pic, bbox_rect)
            offset = (pic_min[0] - anim.hotspot[0], pic_min[1] - anim.hotspot[1])
            new_anim.append_frame([pywi.animation.Blit(chunk, offset)])
        new_animations[name] = new_anim
    return new_animations

def optimize_greedy(animations, chunksets):
    print 'Running greedy optimization...'
    new_animations = {}
    for name, anim in animations.iteritems():
        print 'Optimizing %s' % (name)
        if chunksets[anim.has_player_color] is None:
            chunksets[anim.has_player_color] = pywi.animation.ChunkSet(anim.has_player_color)
        chunkset = chunksets[anim.has_player_color]
        new_anim = pywi.animation.AnimationBlits(chunkset)
        new_anim.options.update(anim.options)

def compute_animations_hash(animations):
    """
    Compute a hash of animation data that should be independent of
    any form of optimization.
    """
    m = md5.new()
    m.update('%d' % len(animations))
    for name in sorted(animations.keys()):
        anim = animations[name]
        m.update('=%s:%s:%d;%s;%d' % (
            name, anim.has_player_color, len(anim.options),
            ':'.join(['%s=%s' % (key, anim.options[key]) for key in sorted(anim.options.keys())]),
            anim.get_nrframes()
        ))
        for idx in xrange(anim.get_nrframes()):
            m.update(':')
            frame = anim.get_frame(idx)
            for y in xrange(frame.pic.shape[0]):
                for x in xrange(frame.pic.shape[1]):
                    if frame.pic[y,x,3] != 0:
                        m.update('.%d,%d.%d.%d.%d.%d' % (
                            y - anim.hotspot[0], x - anim.hotspot[1],
                            frame.pic[y,x,0], frame.pic[y,x,1], frame.pic[y,x,2], frame.pic[y,x,3]
                        ))
                        if frame.pc_pic is not None:
                            m.update('.%d.%d.%d' % (frame.pc_pic[y,x,0], frame.pc_pic[y,x,1], frame.pc_pic[y,x,2]))
    return m.hexdigest()

def add_animation(arg):
    try:
        m = re.match(r'(\w+),(\d+),(\d+)$', arg)
        return (m.group(1), int(m.group(3)), int(m.group(2)))
    except:
        raise argparse.ArgumentTypeError('must be of the form "<name>,<x>,<y>", where x,y is the hotspot')

# NOCOM(#sirver): support for dirpics.
def parse_args():
    p = argparse.ArgumentParser(description=
        """
        Transform the animation pictures found in the given directory
        into a (possibly optimized) spritemap.
        """
    )

    p.add_argument(
        'directory', type=str, default='.',
        help=''
    )
    p.add_argument(
        '-a', '--add', action='append', type=add_animation, metavar='name,x,y',
        default=[],
        help='Add an animation of the given name and with the given hotspot from <name>_??.png and <name>_??_pc.png files'
    )
    p.add_argument(
        '-o', '--optimize', type=str, choices=['bbox', 'greedy'], default='bbox',
        help="Frame optimization routine ('bbox' is very fast, but 'greedy' can give better results)"
    )
    p.add_argument(
        '-d', '--dry-run', action='store_true',
        help="Perform all optimizations, but do not write the final result"
    )

    return p.parse_args()

def error(msg):
    print >>sys.stderr, msg
    sys.exit(1)

def main():
    args = parse_args()

    animations = {}
    if os.path.exists(args.directory + '/conf'):
        with open(args.directory + '/conf', 'r') as filp:
            print 'Loading existing conf file...'
            conf = pywi.config.read(filp)
        for name, section in conf.itersections():
            if 'pics' not in section and 'spritemap' not in section:
                continue
            if name in animations:
                error('conf file contains multiply defined animation')
            print 'Loading animation %s...' % (name)
            animations[name] = pywi.animation.load_section(args.directory, section)
    else:
        conf = pywi.config.File()

    for name, y, x in args.add:
        if name in animations:
            error('Trying to add animation %s, but name already exists' % (name))
        print 'Loading added animation %s...' % (name)
        anim = pywi.animation.load_glob(args.directory + '/%s_??.png' % (name))
        anim.hotspot = (y,x)
        animations[name] = anim

    if not animations:
        print 'No animations loaded.'
        sys.exit()

    orighash = compute_animations_hash(animations)
    origcost = sum([anim.get_cost(FRAGMENT_COST) for anim in animations.itervalues()])
    print 'Loaded %d animation with %d frames of cost %d, hash %s' % (
        len(animations),
        sum([anim.get_nrframes() for anim in animations.itervalues()]),
        origcost, orighash
    )

    chunksets = [None, None]
    if args.optimize == 'bbox':
        animations = optimize_bbox(animations, chunksets)
    elif args.optimize == 'greedy':
        animations = optimize_greedy(animations, chunksets)
    else:
        error('Unknown optimization method %s' % (arg.optimize))

    newhash = compute_animations_hash(animations)
    newcost = 0
    for chunkset in chunksets:
        if chunkset is not None:
            newcost += chunkset.get_cost(FRAGMENT_COST)
    print 'Resulting animations of cost %d, hash %s' % (newcost, newhash)
    if newhash != orighash:
        print 'ERROR: Animations incorrectly modified'
        sys.exit(1)

    for idx, chunkset in enumerate(chunksets):
        if chunkset is None:
            continue
        rects = [(chunk.pic.shape[0], chunk.pic.shape[1]) for chunk in chunkset.chunks]
        chunk_area = sum([r[0] * r[1] for r in rects])
        print 'Packing chunks of area %d pixels' % (chunk_area)
        ext0, ext1, offsets = pywi.packing.pack(rects)
        packed_area = ext0 * ext1
        overhead = float(packed_area - chunk_area) / chunk_area * 100
        print '  packed into %dx%d = %d pixels (%.1f%% overhead)' % (
            ext1, ext0, packed_area, overhead)
        chunkset.assign_packing(ext0, ext1, offsets)

        if not args.dry_run:
            n = 0
            while (os.path.exists(args.directory + '/spritemap' + str(n) + '.png') or
                os.path.exists(args.directory + '/spritemap' + str(n) + '_pc.png')):
                n += 1

            spritemap = 'spritemap%d' % (n)
            print 'Writing spritemap file %s.png...' % (spritemap)
            chunkset.write_images(args.directory, spritemap)

    if not args.dry_run:
        for name, anim in animations.iteritems():
            print 'Writing animation %s...' % (name)
            s = conf.make_section(name)
            anim.write(args.directory, s)

        with open(args.directory + '/conf', 'w') as filp:
            print 'Writing conf data to %s...' % (args.directory + '/conf')
            conf.write(filp)

if __name__ == '__main__':
    main()
