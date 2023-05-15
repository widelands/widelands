-- Returns definitions for encyclopedia tabs and their contents for the Tribal Encyclopedia

include "scripting/help.lua"

-- Comparison function used to sort map objects alphabetically
function compare_by_title(a, b)
  return a["title"] < b["title"]
end

-- Helper function to return all entries of a certain type for the tribe
function map_object_entries(tribename, script_filename, map_object_table)
   local result = {}
   local entries = {}
   local counter = 1
   for i, map_object in ipairs(map_object_table) do
      -- we need to filter out double entries of Amazon trees
      if not entries[map_object.descname] or not (map_object.type_name == "immovable" and map_object.terrain_affinity) then
         local t = map_object.descname
         if map_object.type_name == "immovable" and map_object:has_attribute("tree") then
            t = map_object.species
         end
         result[counter] = {
            name = map_object.name,
            title = t,
            icon = map_object.icon_name,
            script = script_filename,
            script_parameters = {[1] = tribename, [2] = map_object.name}
         }
         entries[map_object.descname] = counter
         counter = counter + 1
      end
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

-- Returns help entries for all the immovables that the tribe can use
function immovable_entries(tribename)
   local tribe = wl.Game():get_tribe_description(tribename)
   return map_object_entries(tribename, "tribes/scripting/help/immovable_help.lua", tribe.immovables)
end

-- Returns help entries for all the immovables of the world that all tribes can use
function world_immovable_entries(tribename)
   local all_immovables = wl.Descriptions().immovable_descriptions
   local tribes_immovables = {}
   for i, tribe in ipairs(wl.Descriptions().tribes_descriptions) do
      for i, t_immo in ipairs(tribe.immovables) do
         tribes_immovables[t_immo.name] = true
      end
   end
   for i = #all_immovables, 1, -1 do
      if tribes_immovables[all_immovables[i].name] or all_immovables[i].size == "none" or
      (not all_immovables[i]:has_attribute("tree") and all_immovables[i].terrain_affinity) or
      all_immovables[i]:has_attribute("field") then
         table.remove(all_immovables, i);
      end
   end
   return map_object_entries(tribename, "tribes/scripting/help/immovable_help.lua", all_immovables)
end

-- Main function
return {
   func = function(tribename, game_type)
      push_textdomain("tribes_encyclopedia")
      local r = {
         title = _("Tribal Encyclopedia"),
         tabs = {
            {
               name = "general",
               -- TRANSLATORS Tab title: General Widelands help
               title = _("General"),
               icon = "images/logos/wl-ico-32.png",
               entries = {
                  {
                     name = "intro",
                     title = _("Introduction"),
                     script = "tribes/scripting/help/introduction.lua",
                     script_parameters = {}
                  },
                  {
                     name = "controls",
                     title = _("Controls"),
                     script = "tribes/scripting/help/controls.lua",
                     script_parameters = {}
                  },
                  {
                     name = "tips",
                     title = _("Tips"),
                     script = "tribes/scripting/help/tips.lua",
                     script_parameters = {tribename, game_type}
                  }
               }
            },
            {
               name = "wares",
               -- TRANSLATORS Tab title: Ware help
               title = _("Wares"),
               icon = "images/wui/buildings/menu_tab_wares.png",
               entries = ware_entries(tribename)
            },
            {
               name = "workers",
               -- TRANSLATORS Tab title: worker help
               title = _("Workers"),
               icon = "images/wui/buildings/menu_tab_workers.png",
               entries = worker_entries(tribename)
            },
            {
               name = "buildings",
               -- TRANSLATORS Tab title: building help
               title = _("Buildings"),
               icon = "images/wui/stats/genstats_nrbuildings.png",
               entries = building_entries(tribename)
            },
            {
               name = "immovables_tribe",
               -- TRANSLATORS Tab title: immovable help
               title = _("Tribes Immovables"),
               icon = "images/wui/encyclopedia_immovables.png",
               entries = immovable_entries(tribename)
            },
            {
               name = "immovables_world",
               -- TRANSLATORS Tab title: world immovable help
               title = _("World Immovables"),
               icon = "images/wui/encyclopedia_world.png",
               entries = world_immovable_entries(tribename)
            },
         }
      }
      pop_textdomain()
      return r
   end
}
