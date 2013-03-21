#!/usr/bin/env python
# encoding: utf-8

from glob import glob
from itertools import chain, combinations, permutations
import re
import argparse
import os
import sys

import Image
import numpy as np
from scipy import ndimage

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

# NOCOM(#sirver): give credit heer

class Packer(object):
    def fit(self, blocks):
        self.root = { "x": 0, "y": 0, "w": blocks[0].w, "h": blocks[0].h, "used": False }
        self.blocks = blocks
        for b in blocks:
            node = self.find_node(self.root, b.w, b.h)
            if node:
                b.node = self.split_node(node, b.w, b.h)
            else:
                b.node = self.grow_node(b.w, b.h);
            assert(b.node)


    def find_node(self, root, w, h):
        if root["used"]:
            return self.find_node(root["right"], w, h) or self.find_node(root["down"], w, h)
        if (w <= root["w"] and h <= root["h"]):
            return root

    def split_node(self, node, w, h):
        node['used'] = True
        node['down'] = { "x": node["x"], "y": node["y"] + h, "w": node["w"], "h": node["h"] - h, "used": False }
        node['right'] = { "x": node["x"] + w, "y": node["y"], "w": node["w"] - w, "h": h, "used": False};
        return node

    def grow_node(self, w, h):
        can_grow_down = w <= self.root["w"]
        can_grow_right = h <= self.root["h"]

        # Grow to stay 'squarish'
        should_grow_right = can_grow_right and (self.root["h"] >= (self.root["w"] + w))
        should_grow_down = can_grow_down and (self.root["w"] >= (self.root["h"] + h))

        if (should_grow_right):
          return self.grow_right(w, h);
        elif (should_grow_down):
          return self.grow_down(w, h);
        elif (can_grow_right):
          return self.grow_right(w, h);
        elif (can_grow_down):
         return self.grow_down(w, h);
        # Damn, no space for this. Should never happen.
        assert(0)

    def grow_right(self, w, h):
        self.root = {
                'used': True,
                'x': 0, 'y': 0, 'w': self.root['w'] + w, 'h': self.root['h'], 'down': self.root,
                'right': { 'x': self.root["w"], 'y': 0, 'w': w, 'h': self.root['h'], 'used': False },
                }
        node = self.find_node(self.root, w, h)
        if node:
            return self.split_node(node, w, h)

    def grow_down(self, w, h):
        self.root = {
                'used': True,
                'x': 0, 'y': 0, 'w': self.root['w'], 'h': self.root['h'] + h, 'right': self.root,
                'down': { 'x': 0, 'y': self.root["h"], 'w': self.root["w"], 'h': h, 'used': False },
                }
        node = self.find_node(self.root, w, h)
        if node:
            return self.split_node(node, w, h)


    def get_result(self):
        img = np.empty((self.root['h'], self.root['w'], 4), np.uint8)
        pc_img = np.empty((self.root['h'], self.root['w'], 4), np.uint8)
        offsets = {}

        for b in self.blocks:
            assert(b.node)
            n = b.node
            x, y = n['x'], n['y']
            img[y:y+b.h,x:x+b.w] = b.img.astype(np.uint8)
            if b.pc_img is not None:
                pc_img[y:y+b.h,x:x+b.w] = b.pc_img.astype(np.uint8)
            else:
                pc_img = None
            for id in b.id:
                offsets[id] = (x, y)
        return offsets, img, pc_img

# TODO: Merge in crop_animations.py script. It does not matter so much anymore though.
def load_animations(anims):
    rv = []
    rv_pc = []

    def _load(fn, seen_shape):
        img = np.asarray(Image.open(fn))
        if seen_shape and seen_shape != rv[0][0].shape:
            print "This file has different dimensions than the others before. Terminating."
            sys.exit(-1)
        return img

    for anim in anims:
        seen_shape = None
        for idx, fn in enumerate(sorted(glob(anim + "??.png"))):
            rv.append((_load(fn, seen_shape), anim, idx))
            if seen_shape is None:
                seen_shape = rv[0][0].shape
            pc_fn = os.path.splitext(fn)[0] + "_pc.png"
            if os.path.exists(pc_fn):
                rv_pc.append(_load(pc_fn, seen_shape))

    # Crop the Images.
    line_x_all_alpha = lambda x,idx: (rv[idx][0][x,:,-1] == 0).all()
    col_x_all_alpha = lambda x,idx: (rv[idx][0][:,x,-1] == 0).all()
    while all(line_x_all_alpha(0,i) for i in range(len(rv))):
        rv = [(img[1:], anim, idx) for img, anim, idx in rv]
        rv_pc = [img[1:] for img in rv_pc]
    while all(line_x_all_alpha(-1,i) for i in range(len(rv))):
        rv = [(img[:-1], anim, idx) for img, anim, idx in rv]
        rv_pc = [img[:-1] for img in rv_pc]
    while all(col_x_all_alpha(0,i) for i in range(len(rv))):
        rv = [(img[:,1:], anim, idx) for img, anim, idx in rv]
        rv_pc = [img[:,1:] for img in rv_pc]
    while all(col_x_all_alpha(-1,i) for i in range(len(rv))):
        rv = [(img[:,:-1], anim, idx) for img, anim, idx in rv]
        rv_pc = [img[:,:-1] for img in rv_pc]

    return rv, rv_pc

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


def pack_animations(anim_sets):
    pics_to_fit = []
    dimensions = {}
    regions = {}
    for anims in anim_sets:
        new_pics_to_fit, anim_regions, w, h = prepare_animations(anims)
        pics_to_fit.extend(new_pics_to_fit)
        for anim in anims:
            dimensions[anim] = (w, h)
            regions[anim] = anim_regions

    pics_to_fit.sort(reverse=True)
    p = Packer()
    p.fit(pics_to_fit)
    offsets_by_id, result_img, pc_result_img = p.get_result()
    return regions, dimensions, offsets_by_id, result_img, pc_result_img

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
                rv.append("%i,%i" % offsets_by_id[id])
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

# NOCOM(#sirver): support for dirpics.
def parse_args():
    p = argparse.ArgumentParser(description=
        "Creates a Spritemap of the animation pictures found in the current directory."
    )

    p.add_argument("-o", "--output", type=str, default=None, help = "Output picture name. Default is <current dir>.png")
    p.add_argument("-f", "--fps", type=str, default=None, help="Specify frames per second for all the animations. This will be outputted into the conf file and is just there to spare typing.")
    p.add_argument("-s", "--hotspot", type=str, default=None, help="Specify hotspot as 'x y' for all the animations. This will be outputted into the conf file and is just there to spare typing.")

    args = p.parse_args()

    # Find the animations in the current directory
    anims = set()
    for fn in glob('*.png'):
        m = re.match(r'(.*?)\d+\.png', fn)
        if m is None: continue
        anims.add(m.group(1))
    args.anim = sorted(anims)

    if args.output is None:
        args.output = os.path.basename(os.getcwd()) + '.png'
    return args

def main():
    args = parse_args()

    best = 10000**2
    best_result = None
    for anim_sets in set_partitions_all_permutation(args.anim):
        print "Trying: ", anim_sets
        regions, dimensions, offsets_by_id, result_img, pc_result_img = pack_animations(anim_sets)
        size = result_img.shape[0]*result_img.shape[1]
        print "Size: %i x %i = %i pixels" % (result_img.shape[1],result_img.shape[0], size)
        if size < best:
            best = size
            best_result = regions, dimensions, offsets_by_id, result_img, pc_result_img

    regions, dimensions, offsets_by_id, result_img, pc_result_img = best_result
    for anim in args.anim:
        output_results(anim, args.output, dimensions[anim], regions[anim], offsets_by_id, args)
    print "Results were appended to conf."

    Image.fromarray(result_img).save(args.output)
    if pc_result_img is not None:
        pc_fn = os.path.splitext(args.output)[0] + "_pc.png"
        Image.fromarray(pc_result_img).save(pc_fn)


if __name__ == '__main__':
    main()


