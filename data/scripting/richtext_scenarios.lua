include "scripting/richtext.lua"

-- RST
-- .. _richtext_scenarios.lua:
--
-- richtext_scenarios.lua
-- ----------------------
--
-- Functions to simplify text formatting in scenarios and make it consistent.
-- Most of these functions are simple wrapper functions for our :ref:`richtext system <wlrichtext>`.
-- This file will include :ref:`richtext.lua` for you for additional formatting functions.

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
--    :arg title: Title of this text. Use empty string if you don't want any.
--    :arg text: The text itself.
--    :returns: the formatted text.

function speech(img, clr, title, text)
   if title ~= "" then
      title = h1(clr, title)
   end

   -- Surround the text with translatable ","
   text = (_'“%s”'):format(text)

   return title .. li_image(img, p(text))
end


-- RST
-- .. function:: paragraphdivider()
--
--    Closes a paragraph and opens a new paragraph. Use this when you format a string with the speech function
--    and need to divide the speech into multiple paragraphs.
--
--    :returns: close_p() .. open_p()

function paragraphdivider()
   return close_p() .. open_p()
end


-- RST
-- .. function:: objective_text(heading, body)
--
--    Provides nice formatting for objective texts.
--
--    :returns: a rich text object that contains the formatted
--       objective text.

function objective_text(heading, body)
   return h2(heading) .. p(body)
end


-- RST
-- .. function:: new_objectives(...)
--
--    Append an objective text with a header to a dialog box in a nice fashion.
--    For displaying objectives with an extra title when an advisor is talking
--
--    Provides nice formatting for objective texts.
--    The following arguments will be parsed:
--
--       - number: the number of objectives described in the body
--       - body: the objective text, e.g. created with function objective_text(heading, body)
--
--    :returns: a rich text object that contains the formatted
--       objective text & title.

function new_objectives(...)
   local sum = 0
   local text = ""
   for idx,obj in ipairs{...} do
      text = text .. obj.body
      sum = sum + obj.number
   end

   local objectives_header = _"New Objective"
   if (sum > 1) then
      objectives_header = _"New Objectives"
   end

   return
      div("width=100%",
         vspace(18) ..
         div("float=left padding_r=6", p(img("images/wui/menus/objectives.png"))) ..
         p_font("", "size=18 bold=1 color=D1D1D1",  vspace(6) .. objectives_header) ..
         vspace(1) .. text
      )
end
