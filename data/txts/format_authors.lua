include "scripting/richtext.lua"

-- Formatting functions for developers and translators

function h1_authors(text)
   return p_font("align=left", "size=24 color=f4a131", text .. vspace(12))
end

function h2_authors(text)
   return h2(text .. vspace(3))
end
