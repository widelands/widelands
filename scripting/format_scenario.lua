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
   text = (_ '“%s”'):format(text)

   local s = ""
   if title then
      s = rt("<p font-size=20 font-weight=bold font-face=DejaVuSerif " ..
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
   local s = ""
   for idx,obj in ipairs{...} do
   	s = rt("<p font-size=10> <br></p>" ..
	   "<p font=DejaVuSerif font-size=18 font-weight=bold font-color=D1D1D1>"
	   .. ngettext("New Objective", "New Objectives", obj.number) .. "</p>")
	   .. obj.body
   end
   return s
end

