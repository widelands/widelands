-- Text to be scrolled in the intro over the splash image
include("scripting/richtext.lua")

push_textdomain("texts")

include("txts/AUTHORS.lua")
include("txts/TRANSLATORS.lua")

local result = list_authors()

-- Only include current language. Full list would have to be scrolled too fast to
-- finish before intro music ends.
if lang ~= nil then
   result = result .. list_language_translators(lang)
end

pop_textdomain()

return { rt(result) }
