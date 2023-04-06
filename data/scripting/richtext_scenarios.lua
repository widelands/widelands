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
--
-- To make these functions available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/richtext_scenarios.lua"
--

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
--    :type img: :class:`string`
--    :arg clr: A valid 6 char hex color to use for the name of this speaker.
--              Best practice is to use `styles.color()` to get a color value from the style
--              manager whenever possible.
--    :type clr: :class:`string`
--    :arg title: Title of this text. Use empty string if you don't want any.
--    :type title: :class:`string`
--    :arg text: The text itself.
--    :type text: :class:`string`
--
--    :returns: the formatted text.

function speech(img, clr, title, text)
   if title ~= "" then
      title = h1(clr, title)
   end

   -- Surround the text with translatable “quotes”
   push_textdomain("widelands")
   text = (_("“%s”")):format(text)
   pop_textdomain()

   return title .. li_image(img, p(text))
end


-- RST
-- .. function:: paragraphdivider()
--
--    Closes a paragraph and opens a new paragraph. Use this when you format
--    a :ref:`richtext<richtext.lua>` string and need to divide it into multiple paragraphs. Just concatenate
--    this function with the Lua string concatenation operator ``..``
--
--    :returns: close_p() .. open_p()
--
--    Example:
--
--    .. code-block:: lua
--
--       include "scripting/richtext.lua"
--       include "scripting/richtext_scenarios.lua"
--       include "scripting/messages.lua"
--
--       local text = p("This is the first paragraph"
--                      .. paragraphdivider() ..
--                      "This is the second paragraph"
--                     )
--
--       campaign_message_box({title = "Window title",
--                             body = text,
--                             w = 200,
--                             h = 150,
--                             position = "top"})
--

function paragraphdivider()
   return close_p() .. open_p()
end


-- RST
-- .. function:: objective_text(heading, body)
--
--    Provides nice formatting for objective texts.
--
--    :arg heading: The heading
--    :type heading: :class:`string`
--    :arg body: The text
--    :type body: :class:`string`
--
--    :returns: a rich text object that contains the formatted
--       objective text.

function objective_text(heading, body)
   return h2(heading) .. p(body)
end
