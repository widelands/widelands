include "scripting/richtext.lua"

-- Formatting functions for developers and translators

function h1_authors(text)
   return styles.as_paragraph("authors_heading_1", text)
end

function h2_authors(text)
   return h2(text .. vspace(3))
end
