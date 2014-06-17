#!/usr/bin/env python
# encoding: utf-8

import argparse
import math
import md5
import os
import re
import subprocess
import sys

from scipy import ndimage
import numpy as np

import pywi.animation
import pywi.config
import pywi.packing


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

def build_frame_group_rectangle_covering(frames, FRAGMENT_COST=FRAGMENT_COST):
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

def build_frame_group_regions(frames):
    """
    Given a list of frame, identify variable subregions
    and split frames into blits accordingly

    Return (avgcost, list of list of ((x, y), pic, pc_pic))
    """
    pc = frames[0].pc_pic is not None
    regions = []

    if len(frames) > 1:
        # Find the regions that are not equal over all frames
        followers = np.asarray([frame.pic for frame in frames[1:]])
        diff = np.any(np.any(frames[0].pic != followers, 3), 0)
        if pc:
            followers_pc = np.asarray([frame.pc_pic for frame in frames[1:]])
            diff = diff | np.any(np.any(frames[0].pc_pic != followers_pc, 3) & followers[:,:,:,3] != 0, 0)

        #TODO: use rectangle covering instead, once it becomes more efficient
        label_img, nlabels = ndimage.label(diff)
        for i in range(1, nlabels + 1):
            ys, xs = np.where(label_img == i)
            regions.append((ys.min(), xs.min(), ys.max() + 1, xs.max() + 1))
    else:
        diff = np.zeros(frames[0].pic.shape[:2], np.bool)

    base_pic_mask = frames[0].pic[:,:,3] != 0 & ~diff
    for region in regions:
        base_pic_mask[region[0]:region[2], region[1]:region[3]] = False

    ys, xs = np.where(base_pic_mask)
    cost = 0
    base_pic_rect = None
    if len(ys) and len(xs):
        base_pic_rect = (ys.min(), xs.min(), ys.max() + 1, xs.max() + 1)
        base_pic_base = frames[0].pic.copy()
        base_pic_base[:,:,3] = np.choose(base_pic_mask, [0, base_pic_base[:,:,3]])
        base_pic = base_pic_base[base_pic_rect[0]:base_pic_rect[2], base_pic_rect[1]:base_pic_rect[3]]
        if pc:
            base_pic_pc = frames[0].pc_pic[base_pic_rect[0]:base_pic_rect[2], base_pic_rect[1]:base_pic_rect[3]]
        else:
            base_pic_pc = None
        cost = rectangle_cost(base_pic_rect)

    newframes = []
    for frame in frames:
        newframe = [((base_pic_rect[0], base_pic_rect[1]), base_pic, base_pic_pc)] if base_pic_rect else []
        for region in regions:
            pic = frame.pic[region[0]:region[2], region[1]:region[3]]
            if pc:
                pc_pic = frame.pc_pic[region[0]:region[2], region[1]:region[3]]
            else:
                pc_pic = None
            newframe.append(((region[0], region[1]), pic, pc_pic))
            cost += rectangle_cost(region)
        newframes.append(newframe)

    return (float(cost) / len(frames), newframes)


def do_optimize_greedy(frames):
    """
    Find a packing of the frame into frame groups.

    Return list of lists of ((x, y), pic, pc_pic), one list for the blits of each frame.

    Frame groups are optimized together, either based on a simple region heuristic,
    or with a slightly more complicated rectangle covering technique.

    We add frames to candidate frame groups greedily as long as the average cost per frame
    decreases. As a heuristic, frames with high pixel overlap are combined first.
    """
    MAXREJECT = 10
    uncovered = [idx for idx in xrange(len(frames))]
    covered_frames = [None for frame in frames]
    framegroups = []
    total_cost = 0

    print 'pack_frames_greedy: packing', len(frames), 'frames'
    while uncovered:
        leader = uncovered[0]
        del uncovered[0]
        print ' start new framegroup with leader', leader,
        avgcost, newframes = build_frame_group_regions([frames[leader]])
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
            new_avgcost, newframes_try = build_frame_group_regions(
                [frames[i] for i in try_frames]
            )
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
                avgcost = new_avgcost
                newframes = newframes_try
                followers.append(trial)
                uncovered.remove(trial)
                rejections = 0

        print ' adding framegroup', [leader] + followers, 'avgcost', avgcost

        for framenr, newframe in zip([leader] + followers, newframes):
            covered_frames[framenr] = newframe

    return covered_frames

#############################################
#############################################
#############################################


def copy_blits_animation(anim, chunksets):
    """
    Copy the given animation (which must be of type AnimationBlits)
    into a new animation using the given chunksets
    """
    if chunksets[anim.has_player_color] is None:
        chunksets[anim.has_player_color] = pywi.animation.ChunkSet(anim.has_player_color)
    chunkset = chunksets[anim.has_player_color]
    return anim.copy(chunkset)


def optimize_bbox(animations, chunksets, args):
    """
    Joint optimization of the given animations using a simple bounding box routine
    """
    print 'Running bbox optimization...'
    new_animations = {}
    for name in sorted(animations.iterkeys()):
        anim = animations[name]
        if chunksets[anim.has_player_color] is None:
            chunksets[anim.has_player_color] = pywi.animation.ChunkSet(anim.has_player_color)
        chunkset = chunksets[anim.has_player_color]
        new_anim = pywi.animation.AnimationBlits(chunkset)
        new_anim.options.update(anim.options)
        if not args.reopt and type(anim) == pywi.animation.AnimationBlits:
            print 'Copying %s' % (name)
            for frame in anim.frames:
                blits = []
                for blit in frame:
                    chunk = chunkset.make_chunk(
                        blit.chunk.pic, blit.chunk.pc_pic,
                        (0,0) + blit.chunk.pic.shape[0:2]
                    )
                    blits.append(pywi.animation.Blit(chunk, blit.offset))
                new_anim.append_frame(blits)
        else:
            print 'Optimizing %s' % (name)
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

def do_crossframe_optimization(animations, chunkset, optimizer):
    """
    Helper function in which animations have already been reduced
    to those that should be (re-)optimized, and either all animations
    are pc or all are non-pc.

    Aligns all frames of all animations, and submits them together
    to the optimizer. The optimizer takes a list of FullFrame tuples
    and returns a list of (offset, pic, pc_pic) tuple lists, one list for each frame.
    """
    pc = chunkset.has_player_color

    # Step 1: Align all frames of all animations
    frame_min = (1000,1000)
    frame_max = (-1000,-1000)
    for anim in animations.itervalues():
        frame_min = (
            min(frame_min[0], -anim.hotspot[0]),
            min(frame_min[1], -anim.hotspot[1])
        )
        frame_max = (
            max(frame_max[0], anim.shape[0] - anim.hotspot[0]),
            max(frame_max[1], anim.shape[1] - anim.hotspot[1])
        )
    shape = (frame_max[0] - frame_min[0], frame_max[1] - frame_min[1])
    hotspot = (-frame_min[0], -frame_min[1])

    frames = []
    for name in sorted(animations.iterkeys()):
        anim = animations[name]
        rect = (
            hotspot[0] - anim.hotspot[0],
            hotspot[1] - anim.hotspot[1],
            hotspot[0] - anim.hotspot[0] + anim.shape[0],
            hotspot[1] - anim.hotspot[1] + anim.shape[1]
        )
        for framenr in xrange(anim.get_nrframes()):
            frame = anim.get_frame(framenr)
            pic = np.zeros(shape + (4,), np.uint8)
            pic[rect[0]:rect[2], rect[1]:rect[3]] = frame.pic
            if pc:
                pc_pic = np.zeros(shape + (4,), np.uint8)
                pc_pic[rect[0]:rect[2], rect[1]:rect[3]] = frame.pc_pic
            else:
                pc_pic = None
            frames.append((name, framenr, pywi.animation.FullFrame(pic, pc_pic)))

    # Step 2: Perform the actual optimization
    optimized = optimizer([frame[2] for frame in frames])

    # Step 3: Recreate animations
    new_animations = {}
    for name, anim in animations.iteritems():
        new_anim = pywi.animation.AnimationBlits(chunkset)
        new_anim.options.update(anim.options)
        new_animations[name] = new_anim

    for oldframe, newframe in zip(frames, optimized):
        blits = []
        for blit in newframe:
            chunk = chunkset.make_chunk(
                blit[1], blit[2],
                (0,0) + blit[1].shape[0:2]
            )
            blits.append(pywi.animation.Blit(chunk, (blit[0][0] - hotspot[0], blit[0][1] - hotspot[1])))
        assert new_animations[oldframe[0]].get_nrframes() == oldframe[1]
        new_animations[oldframe[0]].append_frame(blits)

    return new_animations

def optimize_greedy(animations, chunksets, args):
    """
    Joint optimization of the given animations, using a greedy set cover
    heuristic to 'cover' animation frames by packed groups.
    """
    new_animations = {}
    if not args.reopt:
        for name in sorted(animations.iterkeys()):
            anim = animations[name]
            if type(anim) == pywi.animation.AnimationBlits:
                print 'Copying %s' % (name)
                new_animations[name] = copy_blits_animation(anim, chunksets)
                del animations[name]

    pc_animations = dict([
        (name, anim) for name, anim in animations.iteritems() if anim.has_player_color
    ])
    nonpc_animations = dict([
        (name, anim) for name, anim in animations.iteritems() if not anim.has_player_color
    ])

    if pc_animations:
        print 'Running greedy optimization for pc animations...'
        if chunksets[True] is None:
            chunksets[True] = pywi.animation.ChunkSet(True)
        new_animations.update(do_crossframe_optimization(
            pc_animations, chunksets[True], do_optimize_greedy
        ))
    if nonpc_animations:
        print 'Running greedy optimization for non-pc animations...'
        if chunksets[False] is None:
            chunksets[False] = pywi.animation.ChunkSet(False)
        new_animations.update(do_crossframe_optimization(
            nonpc_animations, chunksets[False], do_optimize_greedy
        ))
    return new_animations

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

# TODO(sirver): support for dirpics
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
        '-i', '--in-place', action='store_true',
        help="Reuse the same filename for new spritemap if a spritemap already exists"
    )
    p.add_argument(
        '-o', '--optimize', type=str, choices=['bbox', 'greedy'], default='bbox',
        help="Frame optimization routine ('bbox' is very fast, but 'greedy' can give better results)"
    )
    p.add_argument(
        '-r', '--reopt', action='store_true',
        help="Re-optimize also those animations that are already in spritemap format"
    )
    p.add_argument(
        '-d', '--dry-run', action='store_true',
        help="Perform all optimizations, but do not write the final result"
    )
    p.add_argument(
        '-b', '--bzr', action='store_true',
        help="Automatically remove old files from Bazaar and add new ones"
    )

    return p.parse_args()

def error(msg):
    print >>sys.stderr, msg
    sys.exit(1)

def main():
    args = parse_args()

    context = pywi.animation.Context()
    animations = {}
    if os.path.exists(args.directory + '/conf'):
        with open(args.directory + '/conf', 'r') as filp:
            print 'Loading existing conf file...'
            conf = pywi.config.read(filp)
        sections_to_remove = []
        for name, section in conf.itersections():
            if 'dirpics' not in section:
                if 'pics' not in section and 'spritemap' not in section:
                    continue
                if name in animations:
                    error('conf file contains multiply defined animation')
                print 'Loading animation %s...' % (name)
                animations[name] = pywi.animation.load_section(args.directory, section, context)
            else:
                print 'Loading legacy diranimations %s_!!...' % (name)
                animations.update(pywi.animation.load_legacy_diranims(args.directory, name, section, context))
                sections_to_remove.append(name)
        for name in sections_to_remove:
            conf.remove_section(name)
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
        animations = optimize_bbox(animations, chunksets, args)
    elif args.optimize == 'greedy':
        animations = optimize_greedy(animations, chunksets, args)
    else:
        error('Unknown optimization method %s' % (arg.optimize))

    # This is perhaps a bit of an excessive way for achieving this,
    # but re-copy all animations in sorted order. The intention of this
    # is to make the resulting spritemaps be more stable when the same
    # directory is re-optimized
    chunksets = [None, None]
    animations = dict([
        (name, copy_blits_animation(animations[name], chunksets))
        for name in sorted(animations.iterkeys())
    ])

    newhash = compute_animations_hash(animations)
    newcost = 0
    for chunkset in chunksets:
        if chunkset is not None:
            newcost += chunkset.get_cost(FRAGMENT_COST)
    print 'Resulting animations of cost %d, hash %s' % (newcost, newhash)
    if newhash != orighash:
        print 'ERROR: Animations incorrectly modified'
        sys.exit(1)

    spritemap_names = set()
    created_files = set()
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
            spritemap = None
            if args.in_place:
                possible_names = context.spritemap_names.difference(spritemap_names)
                if possible_names:
                    spritemap = possible_names.pop()

            if spritemap is None:
                n = 0
                while (os.path.exists(args.directory + '/spritemap' + str(n) + '.png') or
                    os.path.exists(args.directory + '/spritemap' + str(n) + '_pc.png')):
                    n += 1
                spritemap = 'spritemap%d' % (n)

            spritemap_names.add(spritemap)
            print 'Writing spritemap file %s.png...' % (spritemap)
            chunkset.write_images(args.directory, spritemap)
            created_files.add(os.path.abspath(args.directory + '/' + spritemap + '.png'))
            if chunkset.has_player_color:
                created_files.add(os.path.abspath(args.directory + '/' + spritemap + '_pc.png'))

    if not args.dry_run:
        for name, anim in animations.iteritems():
            print 'Writing animation %s...' % (name)
            s = conf.make_section(name)
            anim.write(args.directory, s)

        with open(args.directory + '/conf', 'w') as filp:
            print 'Writing conf data to %s...' % (args.directory + '/conf')
            conf.write(filp)

        if args.bzr:
            read_files = set([
                os.path.abspath(fn) for fn in context.filenames
            ])
            print 'Adding and removing files from Bazaar...'
            remove_files = read_files.difference(created_files)
            if remove_files:
                subprocess.call(['bzr', 'remove'] + list(remove_files))
            add_files = created_files.difference(read_files)
            if add_files:
                subprocess.call(['bzr', 'add'] + list(add_files))


if __name__ == '__main__':
    main()
