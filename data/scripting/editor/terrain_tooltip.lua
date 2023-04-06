-- RST
-- terrain_tooltip.lua
-- -------------------
--
-- This script returns a compact formatted entry for terrain tooltips in the editor.
-- Pass the internal terrain name to the coroutine to select the terrain type.

include "scripting/richtext.lua"
include "scripting/help.lua"

return {
   func = function(terrain_name)
      push_textdomain("widelands_editor")
      local terrain = wl.Editor():get_terrain_description(terrain_name)

      local result = ""

      -- Trees
      local tree_list = tree_affinity_list(terrain)

      local tree_string = ""
      for k,v in ipairs(tree_list) do
         if v.probability > 0.19 then
            tree_string = tree_string .. div("width=33%", p("valign=center",
               img_object(v.tree.name) .. _("%2.1f%%"):bformat(100 * v.probability) .. space()))

            -- We have space for 3 trees per row in tooltips
            if k % 3 == 0 then
               tree_string = tree_string .. "<br>"
            end
         end
      end

      if (tree_string ~="") then
      -- TRANSLATORS: A header in the editor help
         result = h3(_("Probability of trees growing")) ..
                  vspace(styles.get_size("help_terrain_tree_header_space_after")) ..
                  p(tree_string)
      else
         result = result .. h3(_("No trees will grow here."))
      end

      pop_textdomain()
      return {
         text = div("width=100%", result)
      }
   end
}
