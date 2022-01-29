-- Returns definitions for encyclopedia tabs and their contents for the
-- Editor Help


-- Comparison function used to sort map objects alphabetically
function compare_by_title(a, b)
  return a["title"] < b["title"]
end

-- Returns help entries for all the terrains
function get_terrains()
   local result = {}
   for i, terrain in ipairs(wl.Descriptions().terrain_descriptions) do
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

-- Returns help entries for all the trees
function get_trees()
   local result = {}
   local counter = 1
   for i, immovable in ipairs(wl.Descriptions().immovable_descriptions) do
      if (immovable:has_attribute("tree")) then
         result[counter] = {
            name = immovable.name,
            title = immovable.species,
            icon = immovable.icon_name,
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
push_textdomain("widelands_editor")
local r = {
   title = _("Editor Help"),
   tabs = {
      {
         name = "general",
         -- TRANSLATORS Tab title: General help with the Widelands Editor
         title = _("General"),
         icon = "images/logos/WL-Editor-32.png",
         entries = {
            {
               name = "intro",
               title = _("Introduction"),
               script = "scripting/editor/editor_introduction.lua",
               script_parameters = {}
            },
            {
               name = "controls",
               title = _("Controls"),
               script = "scripting/editor/editor_controls.lua",
               script_parameters = {}
            },
            {
               name = "tips",
               title = _("Tips"),
               script = "scripting/editor/tips.lua",
               script_parameters = {}
            }

         }
      },
      {
         name = "terrains",
         -- TRANSLATORS Tab title: terrain help
         title = _("Terrains"),
         icon = "images/wui/editor/tools/terrain.png",
         entries = get_terrains()
      },
      {
         name = "trees",
         -- TRANSLATORS Tab title: tree help
         title = _("Trees"),
         icon = "world/immovables/trees/alder/menu.png",
         entries = get_trees()
      }
   }
}
pop_textdomain()
return r
