from libc.stdint cimport *
from libcpp.string cimport string as cppstr
from libcpp.set cimport set as cppset

cdef extern from "SDL.h":
    struct SDL_Surface:
        uint32_t flags
        void *format
        int w, h
        uint16_t pitch
        void *pixels
    void SDL_FreeSurface(SDL_Surface*)

cdef extern from "rt_render.h" namespace "RT":
    cdef cppclass IRefMap:
        cppstr query(int16_t, int16_t)
    cdef cppclass IRenderer:
        SDL_Surface * render(char*, uint32_t, IRefMap **, cppset[cppstr] &) except +

    struct IImageLoader:
        pass
    struct IFontLoader:
        pass
    IRenderer * setup_renderer(IFontLoader*, IImageLoader*) except +

cdef extern from "sdl_ttf_font.h" namespace "RT":
    IFontLoader * ttf_fontloader_from_file()
cdef extern from "lodepng_image_loader.h":
    IImageLoader * setup_lodepng_img_loader()

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
        cdef IFontLoader * fl = ttf_fontloader_from_file()
        cdef IImageLoader * imgl = setup_lodepng_img_loader()
        self._renderer = setup_renderer(fl, imgl)

    def __del__(self):
        del self._renderer

    def render(self, text, width, allowed_tags = set()):
        cdef IRefMap * rm
        cdef cppset[cppstr] allowed_set

        for tag in allowed_tags:
            allowed_set.insert(cppstr(<char*>(tag)))

        cdef SDL_Surface * rv = self._renderer.render(text, width, &rm, allowed_set)

        a = np.empty((rv.h, rv.w, 4), np.uint8)
        cdef uint32_t x, y, i
        for y in range(rv.h):
            for x in range(rv.w):
                for i in range(4):
                    a[y,x,i] = (<char*>(rv.pixels))[y*rv.pitch+x*4+i]

        SDL_FreeSurface(rv)

        cdef RefMap rrm = RefMap()
        rrm._refmap = rm

        return a, rrm



