include "scripting/richtext.lua"

-- Formatting functions for developers and translators

function h1_authors(text)
   return styles.as_paragraph("authors_heading_1", text)
end

h2_authors = h2
