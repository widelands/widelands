-- RST
-- formatting.lua
-- --------------
--
-- Functions to simplify and unique text formatting in scenarios and help files.
-- Most of these functions are simple wrapper functions that make working with
-- widelands rich text formatting system more bearable.


-- RST
-- .. function:: localize_list(items, listtype)
--
--    Turns an array of string items into a localized string list with
--    appropriate concatenation.
--
--    e.g. localize_list({"foo", "bar", baz"}, "or") will return _"foo, bar or baz"
--
--    :arg items:    An array of strings
--    :arg listtype: The type of concatenation to use.
--                   Legal values are "&", "and", "or", and ";"
--    :returns: The concatenated list string, using localized concatenation operators.
--
-- Same algorithm as in src/base/i18n
function localize_list(items, listtype)
   local result = ""
   for i, item in pairs(items) do
      if (i == 1) then
         result = item
      elseif (i == #items) then
         if (listtype == "&") then
            result = _"%1$s & %2$s":bformat(result, item)
         elseif (listtype == "or") then
            result = _"%1$s or %2$s":bformat(result, item)
         elseif (listtype == ",") then
            result = _"%1$s, %2$s":bformat(result, item)
         else
            result = _"%1$s and %2$s":bformat(result, item)
      end
      else
         result = _"%1$s, %2$s":bformat(result, item)
      end
   end
   return result
end


-- RST
-- .. function:: rt(text_or_options[, text = nil])
--
--    Wraps a block of text into Lua rich text: <rt>%s</rt>.
--
--    :arg text_or_options: either the text of this rich text
--       or any options you care to give it (e.g. image=pic.png).
--    :type text_or_options: :class:`string`
--    :arg text: if text_or_options is given, this will be the text of the
--       rich text.
--    :returns: the wrapped rich text.
--
function rt(text_or_opts, text)
   local k = "<rt>"
   if text then
      k = ("<rt %s>"):format(text_or_opts)
   else
      text = text_or_opts
   end

   return k .. text .. "</rt>"
end


-- RST
-- .. function:: title(s)
--
--    Returns a paragraph formatted as a center-aligned green title heading with a small gap after it.
--
--    :returns: An rt section with s formatted as a title.
function title(s)
   return "<rt text-align=center><p font-size=28 font-decoration=bold font-face=serif font-color=2F9131>" .. s .. "</p></rt>"
end

-- RST
-- .. function:: h1(s)
--
--    Returns a paragraph formatted as a big heading with a small gap after it.
--    The mnemonic comes from HTML.
--
--    :returns: A paragraph with s formatted as heading.
function h1(s)
   return "<p font-size=18 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=8> <br></p>"
end

-- RST
-- .. function:: h2(s)
--
--    Like :func:`h1` but smaller.
--
--    :returns: A paragraph with s formatted as heading.
function h2(s)
   return "<p font-size=2> <br></p><p font-size=14 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

-- RST
-- .. function:: h3(s)
--
--    Like :func:`h2` but smaller.
--
--    :returns: A paragraph with s formatted as heading.
--
function h3(s)
   return "<p font-size=13 font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

-- RST
-- .. function:: h4(s)
--
--    Like :func:`h3` but smaller.
--
--    :returns: A paragraph with s formatted as heading.
--
function h4(s)
   return "<p font-size=12 font-style=italic font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

-- RST
-- .. function:: p(text_or_options[, text = nil])
--
--    Returns one paragraph with text followed by a small vertical gap. Options
--    can be given as first argument similar to :func:`rt`, otherwise the
--    default options will be :const:`line-spacing=3 font-size=12`.
--
--    :returns: The text wrapped in <p>%s</p>
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

-- RST
-- .. function:: a(text)
--
--    Underlines the text to show links. Only words with default paragraph text style.
--
--    :returns: Underlined text within a default paragraph
function a(text)
   return "</p><p font-size=12 font-decoration=underline>" .. text .. "</p><p font-size=12>"
end

-- RST
-- .. function:: b(text)
--
--    Makes the text bold. Only words with default paragraph text style.
--
--    :returns: Bold text within a default paragraph
function b(text)
   return "</p><p font-size=12 font-weight=bold>" .. text .. "</p><p font-size=12>"
end

-- RST
-- .. function:: i(text)
--
--    Makes the text italic. Only words with default paragraph text style.
--
--    :returns: Italic text within a default paragraph
function i(text)
   return "</p><p font-size=12 font-style=italic>" .. text .. "</p><p font-size=12>"
end

-- RST
-- .. function:: listdivider()
--
--    Closes a paragraph and opens a new paragraph.
--    Use this before starting a list when it doesn't create a paragraph.
--    If you want more space, before the list, use paragraphdivider().
--
--    :returns: <br></p><p font-size=4><br></p><p line-spacing=3 font-size=12>
function listdivider()
   return ("<br></p><p font-size=4><br></p><p line-spacing=3 font-size=12>")
end


-- RST
-- .. function:: listitem(symbol, text)
--
--    Adds the symbol in front of the text to create a list item and adds a paragraphdivider
--
--    :arg symbol: the item symbol for the list, e.g. "•" or "→"
--    :arg text: the text of the list item
--
--    :returns: symbol .. " " .. text .. paragraphdivider()
function listitem(symbol, text)
   return symbol .. " " .. text .. listdivider()
end

-- RST
-- .. function:: listitem_bullet(text)
--
--    Creates a list item with an arrow
--
--    :arg text: the text of the list item
--
--    :returns: listitem("→", text)
function listitem_arrow(text)
   return listitem("→", text)
end

-- RST
-- .. function:: listitem_bullet(text)
--
--    Creates a list item with a bullet point
--
--    :arg text: the text of the list item
--
--    :returns: listitem("•", text)
function listitem_bullet(text)
   return listitem("•", text)
end

-- RST
-- .. function:: dl(dt, dd)
--
--    Creates a description list item
--
--    :arg dt: description term, will be shown in bold
--    :arg dd: description data - the text of the list item
--
--    :returns: b(dt) .. " " .. dd .. "<br>"
function dl(dt, dd)
   return b(dt) .. " " .. dd .. "<br>"
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
