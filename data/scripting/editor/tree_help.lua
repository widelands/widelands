-- RST
-- tree_help.lua
-- -------------
--
-- This script returns a formatted entry for the tree help in the editor.
-- Pass the internal tree name to the coroutine to select the tree type.

include "scripting/formatting.lua"
include "scripting/help.lua"

return {
   func = function(tree_name)
      set_textdomain("widelands_editor")
      local world = wl.World();
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = picture_li(tree.representative_image, "")

      -- TRANSLATORS: A header in the editor help. Terrains preferred by a type of tree.
      result = result .. spacer() .. rt(h2(_"Preferred terrains")) .. spacer()
      result = result .. terrain_affinity_help(tree)

      return {
         title = tree.species,
         text = result
      }
   end
}
