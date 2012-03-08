-- RST
-- formatting.lua
-- --------------
--
-- Function to simplify and unique text formatting in scenarios.  Most of these
-- functions are simple wrapper functions that make working with widelands rich
-- text formatting system more bearable.

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
-- .. function:: h1(s)
--
--    Returns a paragraph formatted as a big heading with a small gap after it.
--    The mnemonic comes from HTML.
--
--    :returns: A paragraph with s formatted as heading.
function h1(s)
   return "<p font-size=18 font-weight=bold font-color=D1D1D1>" --font-face=FreeSerif
      ..  s .. "<br></p><p font-size=8> <br></p>"
end

-- RST
-- .. function:: h2(s)
--
--    Like :func:`h1` but smaller.
--
--    :returns: A paragraph with s formatted as heading.
function h2(s)
   return "<p font-size=14 font-weight=bold font-color=D1D1D1>" --font-face=FreeSerif
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
   return "<p font-size=13 font-color=D1D1D1>" --font-face=FreeSerif font-style=italic
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
   return "<p font-size=12 font-style=italic font-color=D1D1D1>" --font-face=FreeSerif
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
-- .. function:: speech(img, clr, title, text)
--
--    Formats a text as spoken by one person in a scenario. Use it as follows:
--
--    .. code-block:: lua
--
--        function jundlina(title, text)
--           return speech("map:princess.png", "2F9131", title, text)
--        end
--
--    :arg img: name of the image to use for this speaker
--    :arg clr: a valid 6 char hex color to use for the name of this speaker
--    :arg title: Title of this text.
--    :arg text: The text itself. If this is nil, :const:`title` is used as text
--       instead and there will not be any title.
--    :returns: the formatted text.
--
function speech(img, clr, g_title, g_text)
   local title, text = g_title, g_text
   if not text then
      title = nil
      text = g_title
   end

   -- Surround the text with translatable ","
   text = (_ '"%s"'):format(text)

   local s = ""
   if title then
      s = rt("<p font-size=20 font-weight=bold font-face=FreeSerif " ..
         ("font-color=%s>"):format(clr) .. title ..
         "</p><p font-size=8> <br></p>"
      )
   end

   return s .. rt(("image=%s"):format(img), p(text))
end


-- RST
-- .. function:: objective_text(heading, body)
--
--    Provides nice formatting for objective texts.
--
--    :returns: a rich text object that contains the formatted
--       objective text.
--
function objective_text(heading, body)
   return rt(h2(heading) .. p(body))
end

