include "scripting/formatting.lua"

-- NOCOM(#codereview): needs documentation
function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>"
      .. p(text) .. "</rt>"
end

-- NOCOM(#codereview): needs documentation
function spacer()
   return rt(p("font-size=3", ""))
end

-- RST
-- .. function text_line(t1, t2[, imgstr = nil])
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
