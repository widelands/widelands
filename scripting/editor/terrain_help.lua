-- NOCOM fill with content
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function(terrain_name)
      set_textdomain("widelands")
      local terrain = wl.Editor():get_terrain_description(terrain_name)
      local result = p(("%2.1f%%"):bformat(100 * terrain:probability_to_grow("alder_summer_old")))
      return rt(result)
   end
}
