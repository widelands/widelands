include "scripting/richtext.lua"

-- RST
-- richtext_scenarios.lua
-- ----------------------
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
--    :arg title: Title of this text. Use empty string if you don't want any.
--    :arg text: The text itself.
--    :returns: the formatted text.
--
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
--    :returns: <br></p><p font-size=8><br></p><p line-spacing=3 font-size=12>
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
--
function objective_text(heading, body)
   return h2(heading) .. p(body)
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
   local text = ""
   for idx,obj in ipairs{...} do
      text = text .. obj.body
      sum = sum + obj.number
   end
   return h1(ngettext("New Objective", "New Objectives", sum)) .. text
end
