-- RST
-- format_editor.lua
-- -----------------

-- Functions used in the ingame editor help windows for formatting the text and pictures.

include "scripting/formatting.lua"

-- RST
-- .. function:: picture_li(imagepath, text)
--
--    Places a paragraph of text to the right of an image

--    :arg imagepath: the full path to the image file
--    :arg text: the text to be placed next to the image
--
--    :returns: the text wrapped in a paragraph and placed next to the image
function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>"
      .. p(text) .. "</rt>"
end

-- RST
-- .. function:: spacer()
--
--    Adds a little space between two paragraphs
--
--    :returns: a small empty paragraph
function spacer()
   return rt(p("font-size=3", ""))
end

-- RST
-- .. function:: text_line(t1, t2[, imgstr = nil])
--
--    Creates a line of h3 formatted text followed by normal text and an image.
--
--    :arg t1: text in h3 format.
--    :arg t2: text in p format.
--    :arg imgstr: image aligned right.
--    :returns: header followed by normal text and image.
--
function text_line(t1, t2, imgstr)
   if imgstr then
      return "<rt text-align=left image=" .. imgstr ..
         " image-align=right><p font-size=13 font-color=D1D1D1>" ..
         t1 .. "</p><p line-spacing=3 font-size=12>" ..
         t2 .. "<br></p><p font-size=8> <br></p></rt>"
   else
      return "<rt text-align=left><p font-size=13 font-color=D1D1D1>" ..
         t1 .. "</p><p line-spacing=3 font-size=12>" ..
         t2 .. "<br></p><p font-size=8> <br></p></rt>"
   end
end
