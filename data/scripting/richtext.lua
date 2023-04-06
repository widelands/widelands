-- TODO(GunChleoc): Wrap all tags and document allowed attributes when we're done

-- RST
--
-- .. _richtext.lua:
--
-- richtext.lua
-- ------------
--
-- Functions to simplify and unique text formatting in scenarios and help files.
-- Most of these functions are simple wrapper functions that make working with the
-- :ref:`widelands rich text formatting system <wlrichtext>` more bearable.
-- Function names generally follow HTML names.
-- We strongly recommend that you make use of these functions rather than hacking
-- the tags manually.
-- If you're writing a scenario, you should also have a look at
-- :ref:`richtext_scenarios.lua`.
--
-- To make these functions available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/richtext.lua"
--
-- - `Blocks and Positioning`_
-- - `Headings and Paragraphs`_
-- - `Text Formatting`_
-- - `Lists`_
-- - `Images`_
-- - `Links`_
-- - `Text Composition`_
-- - `Code Example`_


-- RST
-- Blocks and Positioning
-- ^^^^^^^^^^^^^^^^^^^^^^
--
-- This section covers functions for structuring your text layout.


-- RST
-- .. function:: rt(text_or_attributes[, text = nil])
--
--    Usually, it is not necessary to wrap your text with this function. If it
--    is missing the backend will take care of it. Wrap your text with this
--    function if you wish to add some attributes to the whole area of text
--    e.g. specifying a different background color for the whole area.
--
--    Wraps a block of text into Lua rich text.
--    Only call this once for the whole text that gets sent to the backend.
--
--    :arg text_or_attributes: see the :ref:`rt tag's documentation <rt_tags_rt>`
--                             for a list of attributes and their descriptions.
--    :type text_or_attributes: :class:`string`
--    :arg text: the text to be enclosed in rich text tags.
--    :type text: :class:`string`
--    :returns: the wrapped rich text.

function rt(text_or_attributes, text)
   if text then
      return "<rt " .. text_or_attributes .. ">" .. text .. "</rt>"
   else
      return "<rt>" .. text_or_attributes .. "</rt>"
   end
end


-- RST
-- .. function:: div(text_or_attributes[, text = nil])
--
--    Wraps a block of text into a div tag.
--
--    :arg text_or_attributes: see the :ref:`div tag's documentation <rt_tags_div>`
--                             for a list of attributes and their descriptions.
--    :type test_or_attributes: :class:`string`
--
--    :arg text: the text to be enclosed in div tags.
--    :type text: :class:`string`
--    :returns: the text wrapped in a div tag.

function div(text_or_attributes, text)
   if text then
      return ("<div %s>"):format(text_or_attributes) .. text .. "</div>"
   else
      return ("<div>") .. text_or_attributes .. "</div>"
   end
end


-- RST
-- .. function:: default_gap()
--
--    Looks up the size of the default gap in the style library.
--
--    :returns: the size of the default gap

function default_gap()
   return styles.get_size(_style_prefix_ .. "text_default_gap")
end


-- RST
-- .. function:: space([gap])
--
--    Adds a horizontal space
--
--    :arg gap: the size of the space as pixels.
--              If omitted, the default gap size will be used.
--
--    :returns: a space tag

function space(gap)
   if gap == nil then
      gap = default_gap()
   end
   return "<space gap="..gap..">"
end


-- RST
-- .. function:: vspace([gap])
--
--    Adds a vertical space
--
--    :arg gap: the size of the space as pixels.
--              If omitted, the default gap size will be used.
--
--    :returns: a vspace tag

function vspace(gap)
   if gap == nil then
      gap = default_gap()
   end
   return "<vspace gap="..gap..">"
end


-- RST
-- .. function:: msg_vspace()
--
--    Adds a standard vertical space for win condition status messages.
--
--    :returns: a vspace tag with the standard gap size

function msg_vspace()
   return vspace(styles.get_size("win_condition_message_gap"))
end

-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- Headings and Paragraphs
-- ^^^^^^^^^^^^^^^^^^^^^^^
--
-- This section covers functions for defining headings and paragraphs.


-- RST
-- .. function:: title(font_face, text)
--
--    Returns a paragraph formatted as a title heading. Use this only for the
--    name of the game.
--
--    :returns: A paragraph with text formatted as title.

function title(font_face, text)
   return styles.as_paragraph("readme_title", font("face=".. font_face, text))
end


-- RST
-- .. function:: pagetitle(text)
--
--    Returns a paragraph formatted as a title heading. Use this on the top of
--    your document only.
--
--    :returns: A paragraph with text formatted as title.

function pagetitle(text)
   return styles.as_paragraph("about_title", text)
end


-- RST
-- .. function:: subtitle(text)
--
--    Returns a paragraph formatted as a subtitle under a title heading.
--    Use this only after title() or pagetitle().
--
--    :returns: A paragraph with text formatted as subtitle.

function subtitle(text)
   return styles.as_paragraph("about_subtitle", text)
end


-- Variable to store current style for text headings and normal paragraphs
if (_style_prefix_ == nil) then
   _style_prefix_ = "wui_"
end

-- RST
-- .. function:: set_fs_style(enable)
--
--    Change the style for text headings and normal paragraphs between the in-game and the main
--    menu styles.
--
--    :arg enable: If evaluates to `true`, then the main menu text style will be used for normal
--                 paragraphs, otherwise the in-game style.

function set_fs_style(enable)
   if enable then
      _style_prefix_ = "fs_"
   else
      _style_prefix_ = "wui_"
   end
end


-- RST
-- .. function:: fs_color(text)
--
--    .. deprecated:: 1.2 Use `set_fs_style()` instead.
--
--    Returns the given text wrapped in a font tag for the
--    default color that is used for texts in the main menu.
function fs_color(text)
   return "<font color=FFDC00>" .. text .. "</font>"
end


-- RST
-- .. function:: h1(text_or_color[, text = nil])
--
--    Returns a paragraph formatted as a big heading with a small gap after it.
--
--    :returns: A paragraph with text formatted as heading.

function h1(text_or_color, text)
   local t
   if text then
      t = font("color=".. text_or_color, text)
   else
      t = text_or_color
   end
   return styles.as_paragraph(_style_prefix_ .. "heading_1", t)
end


-- RST
-- .. function:: h2(text)
--
--    Like :func:`h1` but smaller.
--
--    :returns: A paragraph with text formatted as heading.

function h2(text)
   return styles.as_paragraph(_style_prefix_ .. "heading_2", text)
end


-- RST
-- .. function:: h3(text)
--
--    Like :func:`h2` but smaller.
--
--    :returns: A paragraph with text formatted as heading.

function h3(text)
   return styles.as_paragraph(_style_prefix_ .. "heading_3", text)
end


-- RST
-- .. function:: h4(text)
--
--    Like :func:`h3` but smaller.
--
--    :returns: A paragraph with text formatted as heading.

function h4(text)
   return styles.as_paragraph(_style_prefix_ .. "heading_4", text)
end


-- RST
-- .. function:: inline_header(header, text)
--
--    Creates a line of h3 formatted text followed by normal paragraph text.
--
--    :arg header: text in h3 format.
--    :arg text: text in p format.
--    :returns: header text followed by normal text.

function inline_header(header, text)
   return
      div("width=100%",
          vspace(styles.get_size(_style_prefix_ .. "text_space_before_inline_header"))) ..
      div("width=100%",
          styles.as_font_from_p(_style_prefix_ .. "heading_3", header .. " ") ..
          styles.as_font_from_p(_style_prefix_ .. "text", text))
end


-- RST
-- .. function:: p(text_or_attributes[, text = nil])
--
--    Returns one paragraph with text followed by a small vertical gap. Options
--    can be given as first argument similar to :func:`rt`.
--
--    :arg text_or_attributes: see the :ref:`p tag's documentation <rt_tags_p>`
--                             for a list of attributes and their descriptions.
--    :type text_or_attributes: :class:`string`
--
--    :returns: The text wrapped in <p>%s</p>

function p(text_or_attributes, text)
   if text then
      return styles.as_p_with_attr(_style_prefix_ .. "text", text_or_attributes, text)
   else
      return styles.as_paragraph(_style_prefix_ .. "text", text_or_attributes)
   end
end


-- RST
-- .. function:: open_p([attributes = nil])
--
--    Returns a paragraph open tag and default font size. Options
--    can be given as first argument similar to :func:`rt`.
--
--    :arg attributes: see the :ref:`p tag's documentation <rt_tags_p>`
--                     for a list of attributes and their descriptions.
--    :type attributes: :class:`string`
--
--    :returns: <p> with added attributes and default font

function open_p(attributes)
   if attributes then
      return styles.open_p_with_attr(_style_prefix_ .. "text", attributes)
   else
      return styles.open_p(_style_prefix_ .. "text")
   end
end


-- RST
-- .. function:: close_p()
--
--    Closes a paragraph.
--
--    :returns: The closing tags for a paragraph

function close_p()
   return styles.close_p(_style_prefix_ .. "text")
end


-- RST
-- .. function:: p_font(p_or_font_attributes, text_or_font_attributes[, text = nil])
--
--    Returns one paragraph with text followed by a small vertical gap. Options
--    can be given as first argument similar to :func:`rt`.
--
--    :arg p_or_font_attributes: Optional paragraph or font attributes.
--    :type p_or_font_attributes: :class:`string`
--
--    :arg text_or_font_attributes: Optional font attributes or the text itself.
--    :type text_or_font_attributes: :class:`string`
--
--    See the :ref:`p tag's documentation <rt_tags_p>` for a list of paragraph
--    attributes and the :ref:`font tag's documentation <rt_tags_font>` for a
--    list of font attributes.
--
--    :returns: The text wrapped in <p attributes><font attributes>text</font></p>

function p_font(p_or_font_attributes, text_or_font_attributes, text)
   if text then
      return p(p_or_font_attributes, font(text_or_font_attributes, text))
   else
      return p(font(p_or_font_attributes, text_or_font_attributes))
   end
end


-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- Text Formatting
-- ^^^^^^^^^^^^^^^
--
-- This section covers convenience functions for text formatting.


-- RST
-- .. function:: font(attributes, text)
--
--    Wraps the text in font tags. See also :any:`p_font`.
--
--    :arg attributes: see the :ref:`font tag's documentation <rt_tags_font>`
--                     for a list of attributes and their descriptions.
--    :type attributes: :class:`string`
--
--    :returns: The text wrapped in font tags with the given attributes

function font(attributes, text)
   return ("<font %s>"):format(attributes) .. text .. "</font>"
end

-- RST
-- .. function:: b(text)
--
--    This makes the text bold.
--
--    :arg text: the text to format
--
--    :returns: a font tag containing the bold text

function b(text)
   return font("bold=1", text)
end

-- RST
-- .. function:: i(text)
--
--    This makes the text italic.
--
--    :arg text: the text to format
--
--    :returns: a font tag containing the italic text

function i(text)
   return font("italic=1", text)
end

-- RST
-- .. function:: u(text)
--
--    This underlines the text.
--
--    :arg text: the text to format
--
--    :returns: a font tag containing the underlined text

function u(text)
   return font("underline=1", text)
end


-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- Lists
-- ^^^^^
--
-- This section covers functions for defining lists.


-- RST
-- .. function:: dl(dt, dd)
--
--    This function imitates a HTML description list
--
--    :arg dt: "description term", will be rendered in bold.
--    :arg dd: "description data", will be rendered normally.
--
--    :returns: a p tag containing the formatted text

function dl(dt, dd)
   return p(b(dt) .. " " .. dd)
end


-- RST
-- .. function:: li(text_or_symbol[, text = nil])
--
--    Adds the symbol in front of the text to create a list item and
--    wraps it in a paragraph
--
--    :arg symbol: the item symbol for the list, e.g. "•" or "→". "•" is the default.
--    :arg text: the text of the list item
--
--    :returns: a p tag containing the formatted text

function li(text_or_symbol, text)
   local symbol
   local t
   if text then
      symbol = text_or_symbol
      t = text
   else
      symbol ="•"
      t = text_or_symbol
   end
   return div(p(symbol)) .. div(p(space())) .. div("width=*", p(t .. vspace()))
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
-- .. function:: li_image(imagepath, text)
--
--    Places a paragraph of text to the right of an image
--
--    :arg imagepath: the full path to the image file
--    :type imagepath: :class:`string`
--
--    :arg text: the text to be placed next to the image
--    :type text: :class:`string`
--
--    :returns: the text wrapped in a paragraph and placed next to the image, the outer tag is a div.

function li_image(imagepath, text)
   return
      div("width=100%",
         div("float=left padding_r=" .. default_gap(), p(img(imagepath))) ..
         p(text)
      )
end

-- RST
-- .. function:: li_object(name, text[, playercolor])
--
--    Places a paragraph of text to the right of an image representing the given map object
--
--    :arg name: the name of the map object to be represented by an image
--    :type name: :class:`string`
--
--    :arg text: the text to be placed next to the image
--    :type text: :class:`string`
--
--    :arg playercolor: a playercolor to be applied to the image, in hex notation
--    :type playercolor: :class:`string`
--
--    :returns: the text wrapped in a paragraph and placed next to the image, the outer tag is a div.

function li_object(name, text, playercolor)
   local image = img_object(name)
   if (playercolor ~= nil) then
      image = img_object(name, "color=" .. playercolor)
   end
   return
      div("width=100%",
         div("float=left padding_r=" .. default_gap(), p(image)) ..
         p(text)
      )
end

-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- Images
-- ^^^^^^
--
-- This section covers functions for including images.


-- RST
-- .. function:: img(src[, attributes = nil])
--
--    Turns an image src path into an image tag for richtext. See also :any:`li_image`.
--
--    :arg src: the file path to the image.
--    :type src: :class:`string`
--    :arg attributes: see the :ref:`img tag's documentation <rt_tags_img>`
--                     for a list of attributes and their descriptions.
--    :type attributes: :class:`string`
--
--    :returns: the img tag.

function img(src, attributes)
   if attributes then
      return "<img src=" .. src .." " .. attributes .. ">"
   else
      return "<img src=" .. src .. ">"
   end
end

-- RST
-- .. function:: img_object(object[, attributes = nil])
--
--    Creates a richtest image tag for the given map object type. See also :any:`li_object`.
--
--    :arg name: the name of the map object.
--    :type name: :class:`string`
--    :arg attributes: see the :ref:`img tag's documentation <rt_tags_img>`
--                     for a list of attributes and their descriptions.
--    :type attributes: :class:`string`
--
--    :returns: the img tag.

function img_object(name, attributes)
   if attributes then
      return "<img object=" .. name .. " " .. attributes .. ">"
   else
      return "<img object=" .. name .. ">"
   end
end


-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- Links
-- ^^^^^
--
-- This section covers functions for including hyperlinks.

-- RST
-- .. function:: a(linktext, linktype, target[, action=nil][, mouseover=target|linktext])
--
--    This function creates a hyperlink.
--
--    :arg linktext: The text to display.
--    :type linktext: :class:`string`
--    :arg linktype: Type of the link. Use ``"ui"`` for internal links and ``"url"`` for external (browser) links.
--    :type linktype: :class:`string`
--    :arg target: The name of the UI element to notify or the URL to open.
--    :type target: :class:`string`
--    :arg action: Only valid for UI links. The action the UI element should perform.
--    :type action: :class:`string` or :class:`nil`
--    :arg mouseover: The text to show on mouse hovering. This defaults to the target for URLs or the linktext for other links; use ``""`` to disable.
--    :type mouseover: :class:`string`
--
--    :returns: The linkified and underlined text.

function a(linktext, linktype, target, action, mouseover)
   local str = "<link type=" .. linktype .. " target=\"" .. target .. "\""
   if action ~= nil then str = str .. " action=\"" .. action .. "\"" end
   return str .. "mouseover=\"" .. (mouseover or ((linktype == "url") and target or linktext)) .. "\">" .. font("underline=1", linktext) .. "</link>"
end

-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- Text Composition
-- ^^^^^^^^^^^^^^^^
--
-- This section covers functions for text composition that help with proper markup
-- to make the text translatable.


-- RST
-- .. function:: join_sentences(sentence1, sentence2)
--
--    Joins 2 sentences together. Use this rather than manually concatenating
--    a blank space, because some languages don't use blank spaces.
--
--    :arg sentence1: text of the first sentence
--    :arg sentence2: text of the second sentence
--    :returns: two concatenated sentences with a localized sentence joiner.

function join_sentences(sentence1, sentence2)
   -- TRANSLATORS: Put 2 sentences one after the other.
   -- TRANSLATORS: Languages using Chinese script probably want to lose the blank space here.
   return pgettext("sentence_separator", "%s %s"):bformat(sentence1, sentence2)
end


-- RST
-- .. function:: localize_list(items, listtype)
--
--    Turns an array of string items into a localized string list with
--    appropriate concatenation.
--
--    e.g. localize_list({"foo", "bar", "baz"}, "or") will return
--    _("foo, bar or baz")
--
--    :arg items:              An array of strings
--    :arg listtype:           The type of concatenation to use.
--                             Legal values are "&", "and", "or", and ","
--    :returns: The concatenated list string, using localized concatenation operators.
--
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


-- RST
-- :ref:`Return to index<richtext.lua>`
--
-- .. _lua_formatting_example:
--
-- Code Example
-- ^^^^^^^^^^^^
-- Here's an example on how these functions and their attributes can be used. The double dot
-- (``..``) is the LUA string concatenation operator. Note that this example
-- also includes translation markup (the ``_([[Some text]])`` or ``_("Some text")`` function):
--
-- .. code-block:: lua
--
--    include "scripting/richtext.lua"
--
--    title = "Text Formatting",
--    body = h1(_([[Normal header]])) ..
--           h1("6699ff", _([[Colored header]])) ..
--           p(_([[Normal paragraph, just with a bit more text to show how it looks like.]])) ..
--           p("align=center", _([[A centered paragraph, just with a bit more text to show how it looks like.]])) ..
--           li_image("images/wui/menus/statistics.png", _([[An image with right aligned text. This is just text to show automatic line breaks and behavior in regard with images]])) ..
--           li(_([[A list item]])) ..
--           li(font("color=6699ff bold=1", _([[Blue and bold]]))) ..
--           li_arrow(_([[A list item with an arrow]])) ..
--           p(_([[A more complicated paragraph with ]]) ..
--              font("color=ffffff", _([[white text ]])) ..
--              _([[and ]]) ..
--              font("italic=1 bold=1", _([[bold italic formatted text.]]))
--           ),
--
--
-- This results in the following for a campaign message box:
--
-- .. image:: images/wlrichtext.png
--    :scale: 100
--    :alt: sample rendering
--    :align: center
--
-- Please try to avoid hardcoding colors and non-default spacing and sizes. Best practice is to use
-- `styles.color()` to get color values and `styles.get_size()` to get values for spacing and
-- other sizes from the style manager whenever possible.
--
-- :ref:`Return to index<richtext.lua>`
