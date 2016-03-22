-- Returns definitions for encyclopedia tabs and their contents for the
-- Editor Help


-- Comparison function used to sort map objects alphabetically
function compare_by_title(a, b)
  return a["title"] < b["title"]
end

-- Returns help entries for all the terrains in the world
function get_terrains()
   local result = {}
   for i, terrain in ipairs(wl.World().terrain_descriptions) do
      result[i] = {
         name = terrain.name,
         title = terrain.descname,
         icon = terrain.representative_image,
         script = "scripting/editor/terrain_help.lua",
         script_parameters = {[1] = terrain.name}
      }
   end
   table.sort(result, compare_by_title)
   return result
end

-- Returns help entries for all the trees in the world
function get_trees()
   local result = {}
   local counter = 1
   for i, immovable in ipairs(wl.World().immovable_descriptions) do
      if (immovable:has_attribute("tree")) then
         result[counter] = {
            name = immovable.name,
            title = immovable.species,
            icon = immovable.representative_image,
            script = "scripting/editor/tree_help.lua",
            script_parameters = {[1] = immovable.name}
         }
         counter = counter  + 1
      end
   end
   table.sort(result, compare_by_title)
   return result
end

-- Main function
set_textdomain("widelands")
return {
   title = _"Editor Help",
   tabs = {
      {
         name = "general",
         title = _"General",
         icon = "images/logos/WL-Editor-32.png",
         entries = {
            {
               name = "intro",
               title = _"Introduction",
               script = "txts/editor_readme.lua",
               script_parameters = {}
            }
         }
      },
      {
         name = "terrains",
         title = _"Terrains",
         icon = "images/wui/editor/editor_menu_tool_set_terrain.png",
         entries = get_terrains()
      },
      {
         name = "trees",
         title = _"Trees",
         icon = "world/immovables/trees/alder/old/idle_0.png",
         entries = get_trees()
      }
   }
}
