include "tribes/scripting/help/format_help.lua"

-- RST
-- ware_help.lua
-- ---------------
--
-- This script returns a formatted entry for the ingame ware help.
-- Pass the internal tribe name and ware name to the coroutine to select the
-- ware type.


--  =======================================================
--  ************* Main ware help functions *************
--  =======================================================

-- RST
-- .. function:: ware_help_general_string(tribe, ware_description)
--
--    Displays general info texts about the ware
--
--    :arg tribe: The :class:`LuaTribeDescription` for a tribe that uses this ware.
--    :arg ware_description: the ware_description from C++.
--    :returns: General info about the ware
--
function ware_help_general_string(tribe, ware_description)
   -- TODO(GunChleoc): Split into purpose and note
   local helptexts = ware_description:helptexts(tribe.name)
   if helptexts["purpose"] ~= nil then
      return h2(_"Purpose") .. li_image(ware_description.icon_name, helptexts["purpose"])
   else
      return img(ware_description.icon_name)
   end
end

-- RST
-- .. function:: ware_help_producers_string(tribe, ware_description)
--
--    Displays the buildings that produce this ware with information about
--    wares consumed in their production programs
--
--    :arg tribe: The :class:`LuaTribeDescription` for a tribe that uses this ware.
--    :arg ware_description: the ware_description from C++.
--    :returns: Info about buildings producing this ware and the production cost.
--
function ware_help_producers_string(tribe, ware_description)
   local result = ""
   for i, building in ipairs(ware_description:producers(tribe.name)) do
      if (tribe:has_building(building.name)) then
         -- TRANSLATORS: Ware Encyclopedia: A building producing a ware
         result = result .. h2(_"Producer")
         result = result .. dependencies({building, ware_description}, building.descname)
         local producing_programs = {}
         if (building.is_mine) then
            -- Find out which programs in the building produce this ware for the mines we skip the real production and take a dummy "encyclopedia" program
            -- TODO(GunChleoc): Reimplement production programs as a proper tree, so that we can get rid of this hack and of the special case for mines.
            for j, program_name in ipairs(building.production_programs) do
               for ware, amount in pairs(building:produced_wares(program_name)) do
                  local consumes = building:consumed_wares_workers(program_name)
                  if (ware_description.name == ware and #consumes > 0) then
                     table.insert(producing_programs, program_name)
                  end
               end
            end
         else
            -- Find out which programs in the building produce this ware
            for j, program_name in ipairs(building.production_programs) do
               for ware, amount in pairs(building:produced_wares(program_name)) do
                  if (ware_description.name == ware) then
                     table.insert(producing_programs, program_name)
                  end
               end
            end
         end

         -- Now collect all wares produced by the filtered programs
         local produced_wares_strings = {}
         local produced_wares_counters = {}
         for j, program_name in ipairs(producing_programs) do
            local produced_wares_amount = {}
            produced_wares_counters[program_name] = 0
            for ware, amount in pairs(building:produced_wares(program_name)) do
               if (produced_wares_amount[ware] == nil) then
                  produced_wares_amount[ware] = 0
               end
               produced_wares_amount[ware] = produced_wares_amount[ware] + amount
               produced_wares_counters[program_name] = produced_wares_counters[program_name] + amount
            end
            local produced_wares_string = ""
            for ware, amount in pairs(produced_wares_amount) do
               local ware_descr = wl.Game():get_ware_description(ware)
               produced_wares_string = produced_wares_string
                  .. help_ware_amount_line(ware_descr, amount)
            end
            produced_wares_strings[program_name] = produced_wares_string
         end
         -- check for doubled entries (identical consumed and produced wares)
         local deduplicated_programs = {}
         for j, prog1_name in ipairs(producing_programs) do
            local duplicate = false
            for i, prog2_name in ipairs(deduplicated_programs) do
               if produced_wares_strings[prog1_name] == produced_wares_strings[prog2_name] and
                     help_consumed_wares_workers(tribe, building, prog1_name) ==
                     help_consumed_wares_workers(tribe, building, prog2_name) then
                  duplicate = true
                  break
               end
            end
            if not duplicate then
               table.insert(deduplicated_programs, prog1_name)
            end
         end
         producing_programs = deduplicated_programs

         -- Now collect the consumed wares for each filtered program and print the program info
         for j, program_name in ipairs(producing_programs) do
            result = result .. help_consumed_wares_workers(tribe, building, program_name)
            if (produced_wares_counters[program_name] > 0) then
               if (produced_wares_counters[program_name] == 1) then
                  -- TRANSLATORS: Ware Encyclopedia: 1 ware produced by a productionsite
                  result = result .. h3(_"Ware produced:")
               else
                  -- TRANSLATORS: Ware Encyclopedia: More than 1 ware produced by a productionsite
                  result = result .. h3(_"Wares produced:")
               end
               result = result .. produced_wares_strings[program_name]
            end
         end
      end
   end
   return result
end

-- RST
-- .. function:: ware_help_consumers_string(tribe, ware_description)
--
--    Displays the buildings that consume this ware and about
--    workers that use this ware as a tool
--
--    :arg tribe: The :class:`LuaTribeDescription` for a tribe that uses this ware.
--    :arg ware_description: the ware_description from C++.
--    :returns: Info about buildings and workers that use this ware
--
function ware_help_consumers_string(tribe, ware_description)
   local result = ""

   -- Now collecting the buildings that consume this ware
   local consumers_string = ""
   local consumers_amount = 0

   for i, building in ipairs(ware_description:consumers(tribe.name)) do
      if (tribe:has_building(building.name)) then
         consumers_string = consumers_string .. dependencies({ware_description, building}, building.descname)
         consumers_amount = consumers_amount + 1
      end
   end

   -- Constructionsite isn't listed with the consumers, so we need a special check
   if (ware_description:is_construction_material(tribe.name)) then
      local constructionsite_description = wl.Game():get_building_description("constructionsite")
      consumers_string = consumers_string
         .. dependencies({ware_description, constructionsite_description}, constructionsite_description.descname)
      consumers_amount = consumers_amount + 1
   end

   -- Now collecting the workers that use this ware as a tool
   local workers_string = ""
   for i, worker in ipairs(tribe.workers) do
      local add_this_worker = false
      for j, buildcost in ipairs(worker.buildcost) do
         if (buildcost ~= nil and buildcost == ware_description.name) then
            add_this_worker = true
            consumers_amount = consumers_amount + 1
            break
         end
      end
      if(add_this_worker) then
         workers_string = workers_string .. image_line(worker.icon_name, 1, p(worker.descname))
      end
   end

   -- Now show consumers (buildings + workers)
   if (consumers_amount > 0) then
      if (consumers_amount == 1) then
         -- TRANSLATORS: Ware Encyclopedia: Heading for 1 building and / or worker that consumes a ware
         result = result .. h2(_"Consumer")
      else
         -- TRANSLATORS: Ware Encyclopedia: A list of buildings and / or workers that consume a ware
         result = result .. h2(_"Consumers")
      end
      if (consumers ~= "") then
         result = result .. consumers_string
      end
      if (workers_string ~= "") then
         result = result .. workers_string
      end
   end
   return result
end


return {
   func = function(tribename, warename)
      push_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      local ware_description = wl.Game():get_ware_description(warename)
      local r = {
         title = ware_description.descname,
         text = ware_help_general_string(tribe, ware_description)
            .. ware_help_producers_string(tribe, ware_description)
            .. ware_help_consumers_string(tribe, ware_description)
      }
      pop_textdomain()
      return r
   end
}
