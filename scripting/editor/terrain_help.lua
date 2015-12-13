-- NOCOM Sort by percentage and localize
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function(terrain_name)
      set_textdomain("widelands")
      local world = wl.World();
      local terrain = wl.Editor():get_terrain_description(terrain_name)

--[[ NOCOM
      local result = picture_li(terrain.representative_image,
			"Fertility:" .. " " .. ("%2.1f%%"):bformat(100 * terrain.fertility) .. "<br>" ..
			"Humidity:" .. " " .. ("%2.1f%%"):bformat(100 * terrain.humidity) .. "<br>" ..
			"Temperature:" .. " " .. terrain.temperature
		)
		]]

		local result = picture_li(terrain.representative_image, "")

      local tree_string = ""
      for i, tree_name in ipairs(world:immovable_descriptions("tree")) do
			local probability = terrain:probability_to_grow(tree_name)
			local tree = wl.Editor():get_immovable_description(tree_name)
			if (probability > 0.01) then
				tree_string = tree_string .. picture_li(tree.representative_image, tree.basename .. ("<br>Probability to grow: %2.1f%%"):bformat(100 * probability))
			end
		end
		if (tree_string ~="") then
			result = result .. rt(h2("Trees that may grow")) .. tree_string
		else
			result = result .. rt(p("No trees will grow here."))
		end
      return result
   end
}
