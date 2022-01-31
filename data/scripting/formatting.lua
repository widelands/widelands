-- TODO(GunChleoc): This is for the legacy font renderer. Remove when we remove the old renderer and break savegame compatibility.

-- Same algorithm as in src/base/i18n
function localize_list(items, listtype)
   push_textdomain("widelands")
   local result = ""
   for i, item in pairs(items) do
      if (i == 1) then
         result = item
      elseif (i == #items) then
         if (listtype == "&") then
            result = _("%1$s & %2$s"):bformat(result, item)
         elseif (listtype == "or") then
            result = _("%1$s or %2$s"):bformat(result, item)
         elseif (listtype == ",") then
            result = _("%1$s, %2$s"):bformat(result, item)
         else
            result = _("%1$s and %2$s"):bformat(result, item)
      end
      else
         result = _("%1$s, %2$s"):bformat(result, item)
      end
   end
   pop_textdomain()
   return result
end

function rt(text_or_opts, text)
   local k = "<rt>"
   if text then
      k = ("<rt %s>"):format(text_or_opts)
   else
      text = text_or_opts
   end

   return k .. text .. "</rt>"
end

function title(s)
   return "<rt text-align=center><p font-size=28 font-decoration=bold font-face=serif font-color=2F9131>" .. s .. "</p></rt>"
end

function h1(s)
   return "<p font-size=18 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=8> <br></p>"
end

function h2(s)
   return "<p font-size=2> <br></p><p font-size=14 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

function h3(s)
   return "<p font-size=13 font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

function h4(s)
   return "<p font-size=12 font-style=italic font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

function p(text_or_opts, text)
   local opts = "line-spacing=3 font-size=12"
   if text then
      opts = text_or_opts
   else
      text = text_or_opts
   end

   return ("<p %s>"):format(opts) .. text .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end

function a(text)
   return "</p><p font-size=12 font-decoration=underline>" .. text .. "</p><p font-size=12>"
end

function b(text)
   return "</p><p font-size=12 font-weight=bold>" .. text .. "</p><p font-size=12>"
end

function i(text)
   return "</p><p font-size=12 font-style=italic>" .. text .. "</p><p font-size=12>"
end

function listdivider()
   return ("<br></p><p font-size=4><br></p><p line-spacing=3 font-size=12>")
end

function listitem(symbol, text)
   return symbol .. " " .. text .. listdivider()
end

function listitem_arrow(text)
   return listitem("→", text)
end

function listitem_bullet(text)
   return listitem("•", text)
end

function dl(dt, dd)
   return b(dt) .. " " .. dd .. "<br>"
end

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

function paragraphdivider()
   return ("<br></p><p font-size=8><br></p><p line-spacing=3 font-size=12>")
end

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

-- .. function:: spacer()
--
--    Adds a little space between two paragraphs
--
--    :returns: a small empty paragraph
function spacer()
   return rt(p("font-size=3", ""))
end
