-- RST
-- tree_tooltip.lua
-- ----------------
--
-- This script returns a compact formatted entry for tree tooltips in the editor.
-- Pass the internal tree name to the coroutine to select the tree type.

include "scripting/richtext.lua"
include "scripting/help.lua"

local icon_gap = styles.get_size("editor_tooltip_icon_gap")

return {
   func = function(tree_name)
      push_textdomain("widelands_editor")
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = ""

      local terrain_list = terrain_affinity_list(tree)

      for k,v in ipairs(terrain_list) do
         if v.probability > 0.19 then
            result = result .. div("width=33%", p("valign=center",
               img(v.terrain.representative_image) .. space(icon_gap) ..
               _("%2.1f%%"):bformat(100 * v.probability)))
            -- We have space for 3 terrains per row in tooltips
            if k % 3 == 0 then
               result = result .. "<br>"
            end
         end
      end

      -- TRANSLATORS: A header in the editor help. Terrains preferred by a type of tree.
      result = h3(_("Preferred terrains")) ..
               vspace(styles.get_size("help_terrain_tree_header_space_after")) ..
               p(result)
      pop_textdomain()
      return {
         text = result
      }
   end
}
