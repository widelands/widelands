from libc.stdint cimport *
from libcpp.string cimport string as cppstr
from libcpp.set cimport set as cppset
cimport cython

cdef extern from "graphic/text/rt_render.h" namespace "RT":
    cdef cppclass IRefMap:
        cppstr query(int16_t, int16_t)
    cdef cppclass IRenderer:
        Surface* render(char*, uint32_t, cppset[cppstr] &) except +
        IRefMap* make_reference_map(char*, uint32_t, cppset[cppstr] &) except +

cdef extern from "render.h":
    IRenderer* setup_standalone_renderer()

cdef extern from "graphic/surface.h":
    cdef cppclass Surface:
        uint16_t get_pitch()
        uint8_t* get_pixels()
        uint16_t height()
        uint16_t width()



cimport numpy as np
import numpy as np

cdef class RefMap(object):
    cdef IRefMap * _refmap

    def __del__(self):
        del self._refmap

    def query(self, x, y):
        return self._refmap.query(x,y).c_str()

cdef class Renderer(object):
    cdef IRenderer * _renderer

    def __init__(self):
        self._renderer = setup_standalone_renderer()

    def __del__(self):
        del self._renderer

    def render(self, text, width, allowed_tags = set()):
        cdef IRefMap * rm
        cdef cppset[cppstr] allowed_set

        for tag in allowed_tags:
            allowed_set.insert(cppstr(<char*>(tag)))

        cdef Surface* surf = self._renderer.render(text, width, allowed_set)
        rm = self._renderer.make_reference_map(text, width, allowed_set)

        a = np.empty((surf.height(), surf.width(), 4), np.uint8)
        cdef uint32_t x, y, i, clr
        cdef uint8_t* pixels=surf.get_pixels()
        for y in range(surf.height()):
            for x in range(surf.width()):
                for i in range(4):
                    a[y,x,i] = pixels[y*surf.get_pitch() + x*4 + i]

        cdef RefMap rrm = RefMap()
        rrm._refmap = rm

        return a, rrm



