"""Algorithms for rectangle packing."""


class Packer(object):
    """The basic algorithm implemented here is 2D bin packing in a tree with
    some cleverness (for example not knowing the prior size of the final image.

    The best description of the algorithm I found is here
    http://codeincomplete.com/posts/2011/5/7/bin_packing/.
    """

    def fit(self, blocks):
        self.root = {'x': 0, 'y': 0, 'w': blocks[
            0].w, 'h': blocks[0].h, 'used': False}
        self.blocks = blocks
        for b in blocks:
            node = self.find_node(self.root, b.w, b.h)
            if node:
                b.node = self.split_node(node, b.w, b.h)
            else:
                b.node = self.grow_node(b.w, b.h)
            assert (b.node)

    def find_node(self, root, w, h):
        if root['used']:
            return self.find_node(root['right'], w, h) or self.find_node(root['down'], w, h)
        if (w <= root['w'] and h <= root['h']):
            return root

    def split_node(self, node, w, h):
        node['used'] = True
        node['down'] = {'x': node['x'], 'y': node['y'] + h,
                        'w': node['w'], 'h': node['h'] - h, 'used': False}
        node['right'] = {'x': node['x'] + w, 'y': node['y'],
                         'w': node['w'] - w, 'h': h, 'used': False}
        return node

    def grow_node(self, w, h):
        can_grow_down = w <= self.root['w']
        can_grow_right = h <= self.root['h']

        assert can_grow_down or can_grow_right

        # Grow to stay 'squarish'
        should_grow_right = can_grow_right and (
            self.root['h'] >= (self.root['w'] + w))
        should_grow_down = can_grow_down and (
            self.root['w'] >= (self.root['h'] + h))

        if (should_grow_right):
            return self.grow_right(w, h)
        elif (should_grow_down):
            return self.grow_down(w, h)
        elif (can_grow_right):
            return self.grow_right(w, h)
        elif (can_grow_down):
            return self.grow_down(w, h)
        # Damn, no space for this. Should never happen.
        assert (0)

    def grow_right(self, w, h):
        self.root = {
            'used': True,
            'x': 0, 'y': 0, 'w': self.root['w'] + w, 'h': self.root['h'], 'down': self.root,
            'right': {'x': self.root['w'], 'y': 0, 'w': w, 'h': self.root['h'], 'used': False},
        }
        node = self.find_node(self.root, w, h)
        if node:
            return self.split_node(node, w, h)

    def grow_down(self, w, h):
        self.root = {
            'used': True,
            'x': 0, 'y': 0, 'w': self.root['w'], 'h': self.root['h'] + h, 'right': self.root,
            'down': {'x': 0, 'y': self.root['h'], 'w': self.root['w'], 'h': h, 'used': False},
        }
        node = self.find_node(self.root, w, h)
        if node:
            return self.split_node(node, w, h)

    def get_result(self):
        return [
            (b.node['x'], b.node['y'])
            for b in self.blocks
        ]


class Block(object):

    def __init__(self, w, h):
        self.w = w
        self.h = h
        self.node = None


def pack(rectangles):
    """Given a list of (w, h) tuples describing rectangle sizes, determine a
    packing of those rectangles.

    Returns a tuple (w, h, offsets), where offsets is a list of (x, y)
    tuples describing where to put the given rectangles.
    """
    p = Packer()
    p.fit([Block(r[0], r[1]) for r in rectangles])
    return (p.root['w'], p.root['h'], p.get_result())
