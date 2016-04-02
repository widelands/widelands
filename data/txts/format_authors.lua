include "scripting/formatting.lua"

-- Formatting functions
function h1_authors(text)
   return "<rt text-align=left><p font-size=24 font-decoration=bold font-face=serif font-color=f4a131>" .. text .. "</p></rt>"
end

function h2_authors(text)
   return "<rt><p font-size=4> <br></p>" .. h2(text) .. "</rt>"
end

function p_authors(person, image)
   return "<rt image=" .. image .. " text-align=left image-align=left><p font-size=12>" .. person .. "</p></rt>"
end
