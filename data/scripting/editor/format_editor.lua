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
