-- NOCOM fill with content
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function(terrain_name)
      set_textdomain("widelands")
      local world = wl.World();
      local terrain = wl.Editor():get_terrain_description(terrain_name)
      local result = ""
      for j, tree in ipairs(world:immovable_descriptions("tree")) do
			local probability = terrain:probability_to_grow(tree)
			if (probability > 0.01) then
				result = result .. p(tree .. ": " .. ("%2.1f%%"):bformat(100 * probability))
			end
		end
      return rt(result)
   end
}
