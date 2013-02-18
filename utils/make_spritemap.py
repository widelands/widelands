#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import argparse
import os
import sys

import Image
import numpy as np
from scipy import ndimage


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
    seen_shape = None

    def _load(fn):
        print "Loading %s" % fn
        img = np.asarray(Image.open(fn))
        if seen_shape and seen_shape != rv[0][0].shape:
            print "This file has different dimensions than the others before. Terminating."
            sys.exit(-1)
        return img

    for anim in anims:
        for idx, fn in enumerate(sorted(glob(anim + "??.png"))):
            rv.append((_load(fn), anim, idx))
            if seen_shape is None:
                seen_shape = rv[0][0].shape
            pc_fn = os.path.splitext(fn)[0] + "_pc.png"
            if os.path.exists(pc_fn):
                rv_pc.append(_load(pc_fn))

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

def pack_animations(anims):
    base_pic = None

    imgs, pc_imgs = load_animations(anims)
    base_pic = imgs[0][0]
    pc_base_pic = pc_imgs[0]

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
    assert(len(regions) > 0)

    pics_to_fit.append(ImageWrapper(
        cut_out_main(base_pic, regions),
        cut_out_main(pc_base_pic, regions) if pc_base_pic is not None else None,
        Rect(0, base_pic.shape[0], 0, base_pic.shape[1]),
        [("base",-1, -1)])
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

        # Find out if some images are exactly the same
        for subimg, pc_img, id in reg_imgs:
            pc_subimg = None
            if pc_img is not None:
                pc_subimg = pc_img[r.top:r.bottom+1, r.left:r.right+1]
            pics_to_fit.append(ImageWrapper(subimg, pc_subimg, r, id))

    pics_to_fit.sort(reverse=True)
    p = Packer()
    p.fit(pics_to_fit)
    offsets_by_id, result_img, pc_result_img = p.get_result()
    return regions, base_pic.shape[1], base_pic.shape[0], offsets_by_id, result_img, pc_result_img

# NOCOM(#sirver): should take a file descriptor
def output_results(anim, img_name, w, h, regions, offsets_by_id):
    print "[%s]" % anim
    print "pics=%s" % img_name
    print "dimensions=%i,%i" % (w,h)
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

    for ridx,r in enumerate(regions):
        print "region_%02i=%i,%i,%i,%i:%s" % (
            ridx, r.left, r.top, r.right - r.left + 1,
            r.bottom - r.top + 1, _find_offsets(anim, ridx)
            )

    print "fps=10"
    print "hotspot=?? ??"
    print

# NOCOM(#sirver): support for dirpics.
def parse_args():
    p = argparse.ArgumentParser(description=
        "Creates a Spritemap of a given set of images."
    )
    p.add_argument("anim", nargs='+', help = "nocom") # NOCOM(#sirver): help
    p.add_argument("-o", "--output", type=str, default=None, help = "nocom") # NOCOM(#sirver): help

    args = p.parse_args()
    if args.output is None:
        args.output = args.anim[0] + "spritemap.png"
    return args

def main():
    args = parse_args()

    regions, w, h, offsets_by_id, result_img, pc_result_img = pack_animations(args.anim)
    for anim in args.anim:
        output_results(anim, args.output, w, h, regions, offsets_by_id)

    Image.fromarray(result_img).save(args.output)
    if pc_result_img is not None:
        pc_fn = os.path.splitext(args.output)[0] + "_pc.png"
        Image.fromarray(pc_result_img).save(pc_fn)


if __name__ == '__main__':
    main()


