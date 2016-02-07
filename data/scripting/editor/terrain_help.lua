-- NOCOM(#codereview): add a comment where this is called from?
include "scripting/editor/format_editor.lua"

return {
   func = function(terrain_name)
      set_textdomain("widelands")
      local world = wl.World();
      local terrain = wl.Editor():get_terrain_description(terrain_name)

      local result = picture_li(terrain.representative_image, "")

      -- Resources
      local valid_resources = terrain.valid_resources_names
      if (#valid_resources > 0) then
         result = result .. spacer() .. rt(h2(_"Resources"))
         if (#valid_resources > 0) then
            -- TRANSLATORS: A header in the editor help
            result = result .. rt(h3(ngettext("Valid Resource:", "Valid Resources:", #valid_resources)))
            for count, resourcename in pairs(valid_resources) do
               local valid_resource = wl.Editor():get_resource_description(resourcename)
               result = result .. picture_li(valid_resource.representative_image, valid_resource.descname)
            end
         end

         local default_resource_name = terrain.default_resource_name
         if (default_resource_name ~= nil) then
            local default_resource = wl.Editor():get_resource_description(default_resource_name)
            -- TRANSLATORS: e.g. "5x Water"
            result = result .. text_line(_"Default:", _"%1%x %2%":bformat(terrain.default_resource_amount, default_resource.descname))
         end
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
         tree_string = tree_string .. picture_li(tree.representative_image, tree.basename .. ("<br>%2.1f%%"):bformat(100 * v.probability_)) .. spacer()
      end

      -- TRANSLATORS: A header in the editor help
      result = result .. spacer() .. rt(h2(_"Probability of trees growing")) .. spacer()

      if (tree_string ~="") then
         result = result .. tree_string
      else
         result = result .. rt(p(_"No trees will grow here."))
      end
      return result
   end
}
