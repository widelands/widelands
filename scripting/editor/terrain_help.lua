-- NOCOM fill with content
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function(terrain_name)
      set_textdomain("widelands")
      local terrain = wl.Editor():get_terrain_description(terrain_name)
      return rt(p(terrain.name))
   end
}
