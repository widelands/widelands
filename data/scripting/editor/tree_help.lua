-- RST
-- tree_help.lua
-- -------------
--
-- This script returns a formatted entry for the tree help in the editor.
-- Pass the internal tree name to the coroutine to select the tree type.

include "scripting/richtext.lua"
include "scripting/help.lua"

return {
   func = function(tree_name)
      push_textdomain("widelands_editor")
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = li_object(tree.name, "")

      -- TRANSLATORS: A header in the editor help. Terrains preferred by a type of tree.
      result = result ..
               vspace(styles.get_size("help_terrain_tree_header_space_before")) ..
               h2(_("Preferred terrains")) ..
               vspace(styles.get_size("help_terrain_tree_header_space_after")) ..
               terrain_affinity_help(tree)

      pop_textdomain()
      return {
         title = tree.species,
         text = result
      }
   end
}
