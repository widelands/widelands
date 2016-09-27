-- Returns definitions for encyclopedia tabs and their contents for the
-- Tribal Encyclopedia

-- Comparison function used to sort map objects alphabetically
function compare_by_title(a, b)
  return a["title"] < b["title"]
end

-- Helper function to return all entries of a certain type for the tribe
function map_object_entries(tribename, script_filename, map_object_table)
   local result = {}
   for i, map_object in ipairs(map_object_table) do
      result[i] = {
         name = map_object.name,
         title = map_object.descname,
         icon = map_object.icon_name,
         script = script_filename,
         script_parameters = {[1] = tribename, [2] = map_object.name}
      }
   end
   table.sort(result, compare_by_title)
   return result
end

-- Returns help entries for all the buildings that the tribe has
function building_entries(tribename)
   local tribe = wl.Game():get_tribe_description(tribename)
   return map_object_entries(tribename, "tribes/scripting/help/building_help.lua", tribe.buildings)
end

-- Returns help entries for all the wares that the tribe can use
function ware_entries(tribename)
   local tribe = wl.Game():get_tribe_description(tribename)
   return map_object_entries(tribename, "tribes/scripting/help/ware_help.lua", tribe.wares)
end

-- Returns help entries for all the workers that the tribe can use
function worker_entries(tribename)
   local tribe = wl.Game():get_tribe_description(tribename)
   return map_object_entries(tribename, "tribes/scripting/help/worker_help.lua", tribe.workers)
end

-- Main function
return {
   func = function(tribename)
      set_textdomain("tribes_encyclopedia")
      return {
         title = _"Tribal Encyclopedia",
         tabs = {
            {
               name = "general",
               -- TRANSLATORS Tab title: General Widelands help
               title = _"General",
               icon = "images/logos/wl-ico-32.png",
               entries = {
                  {
                     name = "intro",
                     title = _"Introduction",
                     script = "tribes/scripting/help/introduction.lua",
                     script_parameters = {}
                  },
                  {
                     name = "controls",
                     title = _"Controls",
                     script = "tribes/scripting/help/controls.lua",
                     script_parameters = {}
                  }
               }
            },
            {
               name = "wares",
               -- TRANSLATORS Tab title: Ware help
               title = _"Wares",
               icon = "images/wui/buildings/menu_tab_wares.png",
               entries = ware_entries(tribename)
            },
            {
               name = "workers",
               -- TRANSLATORS Tab title: worker help
               title = _"Workers",
               icon = "images/wui/buildings/menu_tab_workers.png",
               entries = worker_entries(tribename)
            },
            {
               name = "buildings",
               -- TRANSLATORS Tab title: building help
               title = _"Buildings",
               icon = "images/wui/stats/genstats_nrbuildings.png",
               entries = building_entries(tribename)
            },
         }
      }
   end
}
