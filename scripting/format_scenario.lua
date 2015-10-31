-- RST
-- format_scenario.lua
-- -------------------
--
-- Function to simplify and unique text formatting in scenarios.  Most of these
-- functions are simple wrapper functions that make working with widelands rich
-- text formatting system more bearable.


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
   text = (_'“%s”'):format(text)

   local s = ""
   if title then
      s = rt("<p font-size=20 font-weight=bold font-face=serif " ..
         ("font-color=%s>"):format(clr) .. title ..
         "</p><p font-size=8> <br></p>"
      )
   end

   return s .. rt(("image=%s"):format(img), p(text))
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
-- .. function:: paragraphdivider()
--
--    Closes a paragraph and opens a new paragraph. Use this when you format a string with the speech function
--    and need to divide the speech into multiple paragraphs.
--
--    :returns: <br></p><p font-size=8><br></p><p line-spacing=3 font-size=12>
function paragraphdivider()
   return ("<br></p><p font-size=8><br></p><p line-spacing=3 font-size=12>")
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


-- RST
-- Append an objective text with a header to a dialog box in a nice fashion.
-- For displaying objectives with an extra title when an advisor is talking
--
--    Provides nice formatting for objective texts.
--    the following arguments will be parsed:
--    number: the number of objectives described in the body
--    body: the objective text, e.g. created with function objective_text(heading, body)
--
--    :returns: a rich text object that contains the formatted
--       objective text & title.
--
function new_objectives(...)
   local sum = 0
   local s = ""
   for idx,obj in ipairs{...} do
      s = s .. obj.body
      sum = sum + obj.number
   end
   return rt("<p font-size=10> <br></p>" ..
      "<p font=serif font-size=18 font-weight=bold font-color=D1D1D1>"
      .. ngettext("New Objective", "New Objectives", sum) .. "</p>") .. s
end