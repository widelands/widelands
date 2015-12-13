include "tribes/scripting/help/format_help.lua"


-- RST
-- ware_help.lua
-- ---------------

-- Functions used in the ingame ware help windows for formatting the text and pictures.


--  =======================================================
--  ************* Main ware help functions *************
--  =======================================================

-- RST
-- .. function ware_help_general_string(tribe, ware_description)
--
--    Displays general info texts about the ware
--
--    :arg tribe: The :class:`LuaTribeDescription` for a tribe that uses this ware.
--    :arg ware_description: the ware_description from C++.
--    :returns: General info about the ware
--
function ware_help_general_string(tribe, ware_description)
   local purpose_text = ware_helptext()
   if (purpose_text ~= "") then
      purpose_text = purpose_text .. " "
   end
   purpose_text = ware_helptext() .. ware_helptext(tribe.name)

   -- TODO(GunChleoc): Split into purpose and note
   local result = rt(h2(_"Purpose")) ..
      rt("image=" .. ware_description.icon_name, p(purpose_text))
   return result
end

-- RST
-- .. function ware_help_producers_string(tribe, ware_description)
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
   for i, building in ipairs(ware_description.producers) do
      if (tribe:has_building(building.name)) then
         -- TRANSLATORS: Ware Encyclopedia: A building producing a ware
         result = result .. rt(h2(_"Producer"))
         result = result .. dependencies({building, ware_description}, building.descname)

         -- Find out which programs in the building produce this ware
         local producing_programs = {}
         for j, program_name in ipairs(building.production_programs) do
            for ware, amount in pairs(building:produced_wares(program_name)) do
               if (ware_description.name == ware) then
                  table.insert(producing_programs, program_name)
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

         -- Now collect the consumed wares for each filtered program and print the program info
         for j, program_name in ipairs(producing_programs) do
            result = result .. help_consumed_wares(building, program_name)
            if (produced_wares_counters[program_name] > 0) then
               result = result
                  -- TRANSLATORS: Ware Encyclopedia: Wares produced by a productionsite
                  .. rt(h3(ngettext("Ware produced:", "Wares produced:", produced_wares_counters[program_name])))
                  .. produced_wares_strings[program_name]
            end
         end

      end
   end
   return result
end

-- RST
-- .. function ware_help_consumers_string(tribe, ware_description)
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

   for i, building in ipairs(ware_description.consumers) do
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
   for i, workername in ipairs(tribe.workers) do
   local worker = wl.Game():get_worker_description(workername)
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
      -- TRANSLATORS: Ware Encyclopedia: A list of buildings and / or workers that consume a ware
      result = result .. rt(h2(ngettext("Consumer", "Consumers", consumers_amount)))
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
   func = function(tribename, ware_description)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      include(ware_description.helptext_script)
      return ware_help_general_string(tribe, ware_description)
         .. ware_help_producers_string(tribe, ware_description)
         .. ware_help_consumers_string(tribe, ware_description)
   end
}
