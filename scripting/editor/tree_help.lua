-- NOCOM Sort by percentage and localize
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function(tree_name)
      set_textdomain("widelands")
      local world = wl.World();
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = picture_li(tree.representative_image, "")
      --[[ NOCOM
      result = result .. rt(h2("Terrain affinity"))
      if (tree.has_terrain_affinity) then
			result = result .. rt(p("Pickiness:" .. " " .. tree.pickiness))
			result = result .. rt(p("Preferred fertility:" .. " " .. ("%2.1f%%"):bformat(100 * tree.preferred_fertility)))
			result = result .. rt(p("Preferred humidity:" .. " " .. ("%2.1f%%"):bformat(100 * tree.preferred_humidity)))
			result = result .. rt(p("Preferred temperature:" .. " " .. tree.preferred_temperature))
      end
      ]]
      result = result .. rt(h2("Preferred terrains"))
      for i, terrain_name in ipairs(world:terrain_descriptions()) do
			local terrain = wl.Editor():get_terrain_description(terrain_name)
			local probability = terrain:probability_to_grow(tree_name)
			if (probability > 0.01) then
				result = result .. rt(h3(terrain.descname))
				result = result .. picture_li(terrain.representative_image, terrain.descname .. "<br>" .. ("%2.1f%%"):bformat(100 * probability))
				--[[ NOCOM
				result = result .. picture_li(terrain.representative_image,
							"Fertility:" .. " " .. ("%2.1f%%"):bformat(100 * terrain.fertility) .. "<br>" ..
							"Humidity:" .. " " .. ("%2.1f%%"):bformat(100 * terrain.humidity) .. "<br>" ..
							"Temperature:" .. " " .. terrain.temperature)
				result = result .. rt(p(("Probability to grow: %2.1f%%"):bformat(100 * probability)))
				]]
			end
		end
      return result
   end
}
