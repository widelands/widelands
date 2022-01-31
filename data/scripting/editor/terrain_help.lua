-- RST
-- terrain_help.lua
-- ----------------
--
-- This script returns a formatted entry for the terrain help in the editor.
-- Pass the internal terrain name to the coroutine to select the terrain type.

include "scripting/richtext.lua"
include "scripting/help.lua"

return {
   func = function(terrain_name)
      push_textdomain("widelands_editor")
      local terrain = wl.Editor():get_terrain_description(terrain_name)

      local result = li_image(terrain.representative_image, "")

      -- Resources
      local valid_resources = terrain.valid_resources
      if (#valid_resources > 0) then
         result = result .. h2(_("Resources"))
         if (#valid_resources > 0) then
            if (#valid_resources == 1) then
               -- TRANSLATORS: A header in the editor help if there is 1 valid resource
               result = result .. h3(_("Valid Resource:"))
            else
               -- TRANSLATORS: A header in the editor help if there is more than 1 valid resource
               result = result .. h3(_("Valid Resources:"))
            end

            for count, resource in pairs(valid_resources) do
               result = result .. li_image(
                  resource.representative_image, resource.descname)
            end
         end

         local default_resource = terrain.default_resource
         if (default_resource ~= nil) then
            result = result .. inline_header(_("Default:"),
               -- TRANSLATORS: e.g. "5x Water"
               _("%1%x %2%"):bformat(terrain.default_resource_amount, default_resource.descname))
         end
      end

      -- Trees
      local tree_list = tree_affinity_list(terrain)

      local tree_string = ""
      for k,v in ipairs(tree_list) do
         tree_string = tree_string .. li_object(v.tree.name,
            v.tree.species .. ("<br>%2.1f%%"):bformat(100 * v.probability)) .. vspace(3)
      end

      -- TRANSLATORS: A header in the editor help
      result = result .. vspace(3) .. h2(vspace(12) .. _("Probability of trees growing")) .. vspace(3)

      if (tree_string ~="") then
         result = result .. tree_string
      else
         result = result .. p(_("No trees will grow here."))
      end

      pop_textdomain()
      return {
         title = terrain.descname,
         text = div("width=100%", result)
      }
   end
}
