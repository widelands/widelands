from libc.stdint cimport *
from libcpp.string cimport string as cppstr
from libcpp.set cimport set as cppset
cimport cython

cdef extern from "thin_graphic.h":
    struct IGraphic:
        pass

cdef extern from "thin_graphic.h":
    cdef cppclass IPixelAccess:
        uint8_t* get_pixels()
        uint16_t get_pitch()
        int lock "lock(IPixelAccess::Lock_Normal)" # DIRTY HACK
        int unlock "unlock(IPixelAccess::Unlock_Update)" # DIRTY HACK
        uint32_t get_h()
        uint32_t get_w()

    cdef cppclass PictureID:
        IPixelAccess& pixelaccess "operator->()->pixelaccess"()
    IGraphic * create_thin_graphic()

cdef extern from "rt_render.h" namespace "RT":
    cdef cppclass IRefMap:
        cppstr query(int16_t, int16_t)
    cdef cppclass IRenderer:
        PictureID render(char*, uint32_t, IRefMap **, cppset[cppstr] &) except +

    struct IFontLoader:
        pass
    IRenderer * setup_renderer(IGraphic, IFontLoader*) except +

cdef extern from "sdl_ttf_font.h" namespace "RT":
    IFontLoader * ttf_fontloader_from_file()

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
        cdef IFontLoader * fl = ttf_fontloader_from_file()
        self._renderer = setup_renderer(thin_graphic[0], fl)

    def __del__(self):
        del self._renderer

    def render(self, text, width, allowed_tags = set()):
        cdef IRefMap * rm
        cdef cppset[cppstr] allowed_set

        for tag in allowed_tags:
            allowed_set.insert(cppstr(<char*>(tag)))

        print "PYALIVE 1"
        cdef PictureID rv = self._renderer.render(text, width, &rm, allowed_set)
        print "PYALIVE 2"
        cdef IPixelAccess* p = &rv.pixelaccess()
        print "PYALIVE 3"

        a = np.empty((p.get_h(), p.get_w(), 4), np.uint8)
        print "PYALIVE 4"
        cdef uint32_t x, y, i, clr
        cdef uint8_t* pixels=p.get_pixels()
        p.lock # DIRTY HACK: Will call lock with correct params
        for y in range(p.get_h()):
            for x in range(p.get_w()):
                for i in range(4):
                    a[y,x,i] = pixels[y*p.get_pitch() + x*4 + i]
        p.unlock # DIRTY HACK: Will call unlock with correct params

        # SDL_FreeSurface(rv)
        # TODO(sirver): Free rv?

        cdef RefMap rrm = RefMap()
        rrm._refmap = rm

        return a, rrm



