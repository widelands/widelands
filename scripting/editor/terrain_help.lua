-- NOCOM localize
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

-- RST
-- .. function text_line(t1, t2[, imgstr = nil])
--
--    Creates a line of h3 formatted text followed by normal text and an image.
--
--    :arg t1: text in h3 format.
--    :arg t2: text in p format.
--    :arg imgstr: image aligned right.
--    :returns: header followed by normal text and image.
--
function text_line(t1, t2, imgstr)
   if imgstr then
      return "<rt text-align=left image=" .. imgstr .. " image-align=right><p font-size=13 font-color=D1D1D1>" ..  t1 .. "</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
   else
      return "<rt text-align=left><p font-size=13 font-color=D1D1D1>" ..  t1 .. "</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
   end
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

		-- Resources
		local default_resource_name = terrain.default_resource_descname
		if (default_resource_name ~= nil) then
			result = result .. rt(h2(_"Resources"))
			-- TRANSLATORS: e.g. "5x Water"
			result = result .. text_line(_"Default Resource:", _"%1%x %2%":bformat(terrain.default_resource_amount, default_resource_name))
		end

		-- Trees
      local tree_list = {}
      for i, tree_name in ipairs(world:immovable_descriptions("tree")) do
			local probability = terrain:probability_to_grow(tree_name)
			if (probability > 0.01) then
				-- sort the trees by percentage
				i = 1
				while (tree_list[i] and (tree_list[i].probability_ > probability)) do
					i = i + 1
				end

				for j = #tree_list, i, -1 do
					tree_list[j+1] = tree_list[j]
				end
				tree_list[i] = {tree_name_ = tree_name, probability_ = probability}
			end
		end

		local tree_string = ""
		for k,v in ipairs(tree_list) do
			local tree = wl.Editor():get_immovable_description(v.tree_name_)
			tree_string = tree_string .. picture_li(tree.representative_image, tree.basename .. ("<br>Probability to grow: %2.1f%%"):bformat(100 * v.probability_))
		end

		if (tree_string ~="") then
			result = result .. rt(h2("Trees that may grow")) .. tree_string
		else
			result = result .. rt(p("No trees will grow here."))
		end
      return result
   end
}
