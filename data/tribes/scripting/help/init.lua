-- NOCOM include "tribes/scripting/help/format_help.lua"
-- NOCOM document

function compare_by_title(a, b)
  return a["title"] < b["title"]
end

function get_buildings(tribename)
   local result = {}
   local tribe = wl.Game():get_tribe_description(tribename)
   for i, buildingname in ipairs(tribe.buildings) do
      local building = wl.Game():get_building_description(buildingname)
      result[i] = {
         name = building.name,
         title = building.descname,
         icon = building.icon_name,
         script = "tribes/scripting/help/building_help.lua",
         script_parameters = {[1] = tribename, [2] = building.name}
      }
   end
   table.sort(result, compare_by_title)
   return result
end
-- NOCOM fix
function get_wares(tribename)
   local result = {}
   local tribe = wl.Game():get_tribe_description(tribename)
   for i, warename in ipairs(tribe.wares) do
      local ware = wl.Game():get_ware_description(warename)
      result[i] = {
         name = ware.name,
         title = ware.descname,
         icon = ware.icon_name,
         script = "tribes/scripting/help/ware_help.lua",
         script_parameters = {[1] = tribename, [2] = ware.name}
      }
   end
   table.sort(result, compare_by_title)
   return result
end
-- NOCOM fix
function get_workers(tribename)
   local result = {}
   local tribe = wl.Game():get_tribe_description(tribename)
   for i, workername in ipairs(tribe.workers) do
      local worker = wl.Game():get_worker_description(workername)
      result[i] = {
         name = worker.name,
         title = worker.descname,
         icon = worker.icon_name,
         script = "tribes/scripting/help/worker_help.lua",
         script_parameters = {[1] = tribename, [2] = worker.name}
      }
   end
   table.sort(result, compare_by_title)
   return result
end

return {
   func = function(tribename)
      set_textdomain("tribes_encyclopedia")
      return {
         title = _"Tribal Encyclopedia",
         tabs = {
            {
               name = "general",
               title = _"General",
               entries = {
                  {
                     name = "intro",
                     title = _"Intro",
                     -- NOCOM icon = "images/wui/buildings/menu_tab_wares.png",
                     script = "txts/README.lua",
                     script_parameters = {}
                  },
                  {
                     name = "abacadabra",
                     title = _"Abacadabra",
                     icon = "images/wui/buildings/menu_tab_wares.png",
                     script = "txts/README.lua",
                     script_parameters = {}
                  }
               }
            },
            {
               name = "wares",
               title = _"Wares",
               icon = "images/wui/buildings/menu_tab_wares.png",
               entries = get_wares(tribename)
            },
            {
               name = "workers",
               title = _"Workers",
               icon = "images/wui/buildings/menu_tab_workers.png",
               entries = get_workers(tribename)
            },
            {
               name = "buildings",
               title = _"Buildings",
               icon = "images/wui/stats/genstats_nrbuildings.png",
               entries = get_buildings(tribename)
            },
         }
      }
   end
}
