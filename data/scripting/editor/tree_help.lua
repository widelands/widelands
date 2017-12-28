-- RST
-- tree_help.lua
-- ---------------
--
-- This script returns a formatted entry for the tree help in the editor.
-- Pass the internal tree name to the coroutine to select the tree type.

include "scripting/richtext.lua"

return {
   func = function(tree_name)
      set_textdomain("widelands_editor")
      local world = wl.World();
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = li_image(tree.representative_image, "")

      -- TRANSLATORS: A header in the editor help. Terrains preferred by a type of tree.
      result = result .. h2(_"Preferred terrains")
      terrain_list = {}
      for i, terrain in ipairs(world.terrain_descriptions) do
         local probability = tree:probability_to_grow(terrain)
         if (probability > 0.01) then
            -- sort the terrains by percentage
            i = 1
            while (terrain_list[i] and (terrain_list[i].probability > probability)) do
               i = i + 1
            end

            for j = #terrain_list, i, -1 do
               terrain_list[j+1] = terrain_list[j]
            end
            terrain_list[i] = {terrain = terrain, probability = probability}
         end
      end

      for k,v in ipairs(terrain_list) do
         result = result .. li_image(v.terrain.representative_image,
               -- TRANSLATORS: Terrain name (Climate)
               (_"%1% (%2%)"):bformat(v.terrain.descname, v.terrain.editor_category.descname) ..
               "<br>" .. ("%2.1f%%"):bformat(100 * v.probability)
            ) .. vspace(3)
      end
      return {
         title = tree.species,
         text = result
      }
   end
}
