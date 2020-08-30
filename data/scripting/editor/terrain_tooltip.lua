-- RST
-- terrain_tooltip.lua
-- -------------------
--
-- This script returns a compact formatted entry for terrain tooltips in the editor.
-- Pass the internal terrain name to the coroutine to select the terrain type.

include "scripting/richtext.lua"

return {
   func = function(terrain_name)
      push_textdomain("widelands_editor")
      local world = wl.World();
      local terrain = wl.Editor():get_terrain_description(terrain_name)

      local result = ""

      -- Trees
      local tree_list = {}
      for i, immovable in ipairs(world.immovable_descriptions) do
         if (immovable:has_attribute("tree")) then
            local probability = immovable:probability_to_grow(terrain)
            if (probability > 0.01) then
               -- sort the trees by percentage
               i = 1
               while (tree_list[i] and (tree_list[i].probability > probability)) do
                  i = i + 1
               end

               for j = #tree_list, i, -1 do
                  tree_list[j+1] = tree_list[j]
               end
               tree_list[i] = {tree = immovable, probability = probability}
            end
         end
      end

      local tree_string = ""
      for k,v in ipairs(tree_list) do
         tree_string = tree_string .. img_object(v.tree.name) ..
             (_("%2.1f%%")):bformat(100 * v.probability) .. space(8)
         -- We have space for 3 trees per row in tooltips
         if k % 3 == 0 then
            tree_string = tree_string .. "<br>"
         end
      end

      if (tree_string ~="") then
      -- TRANSLATORS: A header in the editor help
         result = vspace(3) .. h3(_"Probability of trees growing") .. vspace(3) .. p(tree_string)
      else
         result = result .. h3(_"No trees will grow here.")
      end

      pop_textdomain()
      return {
         text = div("width=100%", result)
      }
   end
}
