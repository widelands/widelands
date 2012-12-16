from libc.stdint cimport *
from libcpp.string cimport string as cppstr
from libcpp.set cimport set as cppset
cimport cython

cdef extern from "thin_graphic.h":
    struct IGraphic:
        pass

cdef extern from "thin_graphic.h":
    cdef cppclass ThinSDLSurface:
        uint8_t* get_pixels()
        uint16_t get_pitch()
        void lock()
        void unlock()
        uint32_t get_h()
        uint32_t get_w()
    ctypedef ThinSDLSurface CThinSDLSurface "const ThinSDLSurface"

    cdef cppclass IPicture:
        pass

    IGraphic * create_thin_graphic()

cdef extern from "rt_render.h" namespace "RT":
    cdef cppclass IRefMap:
        cppstr query(int16_t, int16_t)
    ctypedef IPicture CIPicture "const IPicture"
    cdef cppclass IRenderer:
        CIPicture* render(char*, uint32_t, cppset[cppstr] &) except +
        IRefMap* make_reference_map(char*, uint32_t, cppset[cppstr] &) except +

    struct IFontLoader:
        pass
    IRenderer * setup_renderer(IGraphic, IFontLoader*) except +

cdef extern from "sdl_ttf_font.h" namespace "RT":
    IFontLoader * ttf_fontloader_from_file(cppstr)

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
        cdef IGraphic * thin_graphic = create_thin_graphic()
        cdef IFontLoader * fl = ttf_fontloader_from_file("../../fonts")
        self._renderer = setup_renderer(thin_graphic[0], fl)

    def __del__(self):
        del self._renderer

    def render(self, text, width, allowed_tags = set()):
        cdef IRefMap * rm
        cdef cppset[cppstr] allowed_set

        for tag in allowed_tags:
            allowed_set.insert(cppstr(<char*>(tag)))

        cdef CIPicture* rv = self._renderer.render(text, width, allowed_set)
        cdef CThinSDLSurface* surf = <CThinSDLSurface*>(rv)
        rm = self._renderer.make_reference_map(text, width, allowed_set)

        a = np.empty((surf.get_h(), surf.get_w(), 4), np.uint8)
        cdef uint32_t x, y, i, clr
        cdef uint8_t* pixels=surf.get_pixels()
        surf.lock()
        for y in range(surf.get_h()):
            for x in range(surf.get_w()):
                for i in range(4):
                    a[y,x,i] = pixels[y*surf.get_pitch() + x*4 + i]
        surf.unlock()

        cdef RefMap rrm = RefMap()
        rrm._refmap = rm

        return a, rrm



