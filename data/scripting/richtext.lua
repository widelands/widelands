-- TODO(GunChleoc): Wrap all tags and document allowed attributes when we're done

-- RST
-- richtext.lua
-- ------------
--
-- Functions to simplify and unique text formatting in scenarios and help files.
-- Most of these functions are simple wrapper functions that make working with
-- widelands rich text formatting system more bearable.
-- Function names generally follow HTML names.


-- RST
-- .. function:: localize_list(items, listtype, former_textdomain)
--
--    Turns an array of string items into a localized string list with
--    appropriate concatenation.
--
--    e.g. localize_list({"foo", "bar", baz"}, "or", "widelands") will return
--    _"foo, bar or baz"
--
--    :arg items:              An array of strings
--    :arg listtype:           The type of concatenation to use.
--                             Legal values are "&", "and", "or", and ";"
--    :arg former_textdomain:  The textdomain to restore after running this function.
--    :returns: The concatenated list string, using localized concatenation operators.
--
-- Same algorithm as in src/base/i18n
function localize_list(items, listtype, former_textdomain)
   set_textdomain("widelands")
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
   set_textdomain(former_textdomain)
   return result
end


-- RST
-- .. function:: rt(text_or_attributes[, text = nil])
--
--    Wraps a block of text into Lua rich text.
--    Only call this once for the whole text that gets sent to the backend.
--    There is no general need to wrap an rt tag around your text,
--    because the backend will take care of it.
--    So, only use this function if you wish to add some attributes to the tag.
--
--    Allowed attributes are:
--       padding, padding_r, padding_l, padding_b, padding_t: NOCOM(GunChleoc): Document
--       background:  a background color or image
--       debug:       add visual debug information and debug log
--       editor_mode: allow multiple blank spaces for editing
--
--    :arg attributes: the attributes for the rt tag.
--    :type attributes: :class:`string`
--    :arg text: the text to be enclosed in rich text tags.
--    :type text: :class:`string`
--    :returns: the wrapped rich text.
--
function rt(text_or_attributes, text)
   if text then
      return "<rt " .. text_or_attributes .. ">" .. text .. "</rt>"
   else
      return "<rt>" .. text_or_attributes .. "</rt>"
   end
end


-- RST
-- .. function:: img(src[, attributes = nil])
--
--    Turns an image src path into an image tag for richtext.
--
--    :arg src: the file path to the image.
--    :type src: :class:`string`
--    :arg attributes: the attributes for the div tag.
--    :type attributes: :class:`string`
--
--    Allowed attributes are:
--       color:     a hex color to be applied to the image's player color mask, if it has any
--       ref:       NOCOM(GunChleoc): I have no idea what it does.
--
--    :returns: the img tag.
--
function img(src, attributes)
   if attributes then
      return "<img src=" .. src .." " .. attributes .. ">"
   else
      return "<img src=" .. src .. ">"
   end
end


function title(font_face, text)
   return p_font("align=center", "size=38 face=".. font_face .. " color=2F9131", text)
end


-- RST
-- .. function:: h1(text_or_color[, text = nil])
--
--    Returns a paragraph formatted as a big heading with a small gap after it.
--    The mnemonic comes from HTML.
--
--    :returns: A paragraph with text formatted as heading.
function h1(text_or_color, text)
   if text then
      return p_font("", "size=18 bold=1 color=".. text_or_color, vspace(6) .. text .. vspace(1))
   else
      return p_font("", "size=18 bold=1 color=D1D1D1", vspace(6) .. text_or_color .. vspace(1))
   end
end

-- RST
-- .. function:: h2(text)
--
--    Like :func:`h1` but smaller.
--
--    :returns: A paragraph with text formatted as heading.
function h2(text)
   return p_font("", "size=14 bold=1 color=D1D1D1", vspace(6) .. text .. vspace(1))
end

-- RST
-- .. function:: h3(text)
--
--    Like :func:`h2` but smaller.
--
--    :returns: A paragraph with text formatted as heading.
--
function h3(text)
   return p_font("", "size=13 color=D1D1D1", vspace(4) .. text .. vspace(1))
end

-- RST
-- .. function:: h4(text)
--
--    Like :func:`h3` but smaller.
--
--    :returns: A paragraph with text formatted as heading.
--
function h4(text)
   return p_font("", "size=12 italic=1 color=D1D1D1", text)
end

-- RST
-- .. function:: p(text_or_attributes[, text = nil])
--
--    Returns one paragraph with text followed by a small vertical gap. Options
--    can be given as first argument similar to :func:`rt`.
--
--    Allowed attributes are documented in the open_p function.
--
--    :returns: The text wrapped in <p>%s</p>
function p(text_or_attributes, text)
   if text then
      return open_p(text_or_attributes) .. text .. close_p()
   else
      return open_p() .. text_or_attributes .. close_p()
   end
end


-- RST
-- .. function:: p(text_or_attributes[, text = nil])
--
--    Returns one paragraph with text followed by a small vertical gap. Options
--    can be given as first argument similar to :func:`rt`.
--
--    Allowed p attributes are documented in the open_p function.
--
--    Allowed font attributes are are documented in the font function.
--
--    :returns: The text wrapped in <p attributes><font attributes>text</font></p>
function p_font(p_or_font_or_attributes, text_or_font_attributes, text)
   if text then
      return ("<p %s>"):format(p_or_font_or_attributes) .. "<font " .. text_or_font_attributes .. ">" .. text .. close_p()
   else
      return "<p><font " .. p_or_font_or_attributes .. ">" .. text_or_font_attributes .. close_p()
   end
end


-- RST
-- .. function:: open_p([attributes = nil])
--
--    Returns a paragraph open tag and default font size. Options
--    can be given as first argument similar to :func:`rt`.
--
--    Allowed attributes are:
--       indent:  indents the first line of the paragraph
--       align:   horizontal alignment (left, center, right)
--       valign:  vertical alignment (top, middle, bottom)
--       spacing: line spacing in pixels
--
--    :returns: <p> with added attributes and default font
function open_p(attributes)
   if attributes then
      return ("<p %s>"):format(attributes) .. "<font size=12>"
   else
      return "<p><font size=12>"
   end
end


-- RST
-- .. function:: close_p(t)
--
--    Closes a paragraph.
--
--    :returns: The closing tags for a paragraph
function close_p(t)
   return vspace(6) .. "</font></p>"
end

-- RST
-- .. function:: font(attributes, text)
--
--    Wraps the text in font tags.
--
--    Allowed attributes are:
--       size:      the font size in pt
--       face:      sans, serif or condensed
--       color:     a hex color
--       bold:      if you add bold=1, the text will be bold
--       italic:    if you add italic=1, the text will be italic
--       underline: if you add underline=1, the text will be underlined
--       shadow:    if you add shadow=1, the text will have a shadow
--       ref:       NOCOM(GunChleoc): I don't know what this does.
--
--    :returns: The text wrapped in font tags with the given attributes
--
function font(attributes, text)
   return ("<font %s>"):format(attributes) .. text .. "</font>"
end

-- RST
-- .. function:: space(gap)
--
--    Adds a horizontal space
--    :arg gap: the size of the space as pixels.
--
--    :returns: a space tag
--
function space(gap)
   return "<space gap="..gap..">"
end

-- RST
-- .. function:: vspace(gap)
--
--    Adds a vertical space
--    :arg gap: the size of the space as pixels.
--
--    :returns: a vspace tag
--
function vspace(gap)
   return "<vspace gap="..gap..">"
end

-- RST
-- .. function:: dl(dt, dd)
--
-- This function imitates a HTML description list
--    :arg dt: "description term", will be rendered in bold.
--    :arg dd: "description data", will be rendered normally.
--
--    :returns: a p tag containing the formatted text
--
function dl(dt, dd)
   return p(b(dt) .. " " .. dd)
end

-- RST
-- .. function:: li(text_or_symbol[, text = nil])
--
--    Adds the symbol in front of the text to create a list item and
--    wraps it in a paragraph
--
--    :arg symbol: the item symbol for the list, e.g. "•" or "→"
--    :arg text: the text of the list item
--
--    :returns: a p tag containint the formatted text
function li(text_or_symbol, text)
   if text then
      return p(text_or_symbol .. " " .. text .. vspace(6))
   else
      return p("• " .. text_or_symbol .. vspace(6))
   end
end

-- RST
-- .. function:: li_arrow(text)
--
--    Creates a list item with an arrow
--
--    :arg text: the text of the list item
--
--    :returns: li("→", text)
function li_arrow(text)
   -- TODO(GunChleoc): Reverse arrow for rtl languages.
   return li("→", text)
end

-- RST
-- .. function li_image(imagepath, text)
--
--    Places a paragraph of text to the right of an image

--    :arg imagepath: the full path to the image file
--    :arg text_width_percent: the percentatge of space that the text will occupy
--    :arg text: the text to be placed next to the image
--
--    :returns: the text wrapped in a paragraph and placed next to the image, The outer tag is a div.
function li_image(imagepath, text_width_percent, text)
   return p("<br>") .. div("width=100%", "") ..
         div(p(img(imagepath))) ..
         div(p(space(6))) ..
         div("width="..text_width_percent.."%", p(text)) ..
         div("width=100%", "")
end

-- RST
-- .. function:: a(link)
--
-- This function imitates a HTML link. We can't do real links yet, so the text just gets underlines.
--    :arg link: the text to format
--
--    :returns: a font tag containing the underlined text
--
function a(link)
   return font("underline=1", link)
end

-- RST
-- .. function:: b(link)
--
-- This makes the text bold.
--    :arg link: the text to format
--
--    :returns: a font tag containing the bold text
--
function b(text)
   return font("bold=1", text)
end

-- RST
-- .. function:: i(link)
--
-- This makes the text italic.
--    :arg link: the text to format
--
--    :returns: a font tag containing the italic text
--
function i(text)
   return font("italic=1", text)
end

-- RST
-- .. function:: u(link)
--
-- This underlines the text.
--    :arg link: the text to format
--
--    :returns: a font tag containing the underlined text
--
function u(text)
   return font("underline=1", text)
end

-- RST
-- .. function:: div(text_or_attributes[, text = nil])
--
--    Wraps a block of text into a div tag.
--
--    :arg attributes: the attributes for the div tag.
--    :type attributes: :class:`string`
--
--    Allowed attributes are:
--       padding, padding_r, padding_l, padding_b, padding_t: NOCOM(GunChleoc): Document
--       margin:     NOCOM(GunChleoc): Document
--       float:      NOCOM(GunChleoc): this does not work yet
--       margin:     inner margin for the div
--       valign:     vertical alignment
--       background: a background color or image
--       width:      the width of the div in pixels or percent
--
--    :arg text: the text to be enclosed in div tags.
--    :type text: :class:`string`
--    :returns: the text wrapped in a div tag.
--
function div(text_or_attributes, text)
   if text then
      return ("<div %s>"):format(text_or_attributes) .. text .. "</div>"
   else
      return ("<div>") .. text_or_attributes .. "</div>"
   end
end
