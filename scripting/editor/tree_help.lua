-- NOCOM localize
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
		terrain_list = {}
      for i, terrain_name in ipairs(world:terrain_descriptions()) do
			local terrain = wl.Editor():get_terrain_description(terrain_name)
			local probability = terrain:probability_to_grow(tree_name)
			if (probability > 0.01) then
				---- NOCOM result = result .. rt(h3(terrain.descname))
				-- sort the terrains by percentage
				i = 1
				while (terrain_list[i] and (terrain_list[i].probability_ > probability)) do
					i = i + 1
				end

				for j = #terrain_list, i, -1 do
					terrain_list[j+1] = terrain_list[j]
				end
				terrain_list[i] = {terrain_name_ = terrain_name, probability_ = probability}
			end
		end

		for k,v in ipairs(terrain_list) do
			local terrain = wl.Editor():get_terrain_description(v.terrain_name_)
			result = result .. picture_li(terrain.representative_image, (_"%1% (%2%)"):bformat(terrain.descname, terrain.editor_category.descname) .. "<br>" .. ("%2.1f%%"):bformat(100 * v.probability_))
		end

				--[[ NOCOM
				result = result .. picture_li(terrain.representative_image,
							"Fertility:" .. " " .. ("%2.1f%%"):bformat(100 * terrain.fertility) .. "<br>" ..
							"Humidity:" .. " " .. ("%2.1f%%"):bformat(100 * terrain.humidity) .. "<br>" ..
							"Temperature:" .. " " .. terrain.temperature)
				result = result .. rt(p(("Probability to grow: %2.1f%%"):bformat(100 * probability)))
				]]
      return result
   end
}
