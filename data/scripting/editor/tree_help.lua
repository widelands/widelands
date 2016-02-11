-- RST
-- tree_help.lua
-- -------------

-- This file returns a formatted entry for the tree help in the editor.

include "scripting/editor/format_editor.lua"

return {
   func = function(tree_name)
      set_textdomain("widelands")
      local world = wl.World();
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = picture_li(tree.representative_image, "")

      -- TRANSLATORS: A header in the editor help. Terrains preferred by a type of tree.
      result = result .. rt(p("font-size=3", "")) .. rt(h2(_"Preferred terrains")) .. spacer()
      terrain_list = {}
      for i, terrain_name in ipairs(world.terrain_descriptions) do
         local probability = tree:probability_to_grow(terrain_name)
         if (probability > 0.01) then
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
         -- TRANSLATORS: Terrain name (Climate)
         result = result .. picture_li(terrain.representative_image,
               (_"%1% (%2%)"):bformat(terrain.descname, terrain.editor_category.descname) ..
               "<br>" .. ("%2.1f%%"):bformat(100 * v.probability_)
            ) .. spacer()
      end
      return result
   end
}
