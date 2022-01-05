include "tribes/scripting/help/format_help.lua"

-- RST
-- .. function:: programs_wares_count(building, ware_description)
--
--    Examines the :ref:`production site programs <productionsite_programs>`
--    and returns three tables.
--
--    :arg building: The :class:`wl.map.BuildingDescription`
--    :arg ware_description: A :class:`wl.map.WareDecription`
--    Returns (in this order):
--    An array of program_names this production site has
--    A table of `{program_name, wares_count}` 
--    A table of `{program_name, formatted_string(s)}

function programs_wares_count(tribe, building, ware_description)
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
   --producing_programs = deduplicated_programs
--    for k,v in pairs(producing_programs) do
--       print("Programs: ", k, v)
--    end
--    for k,v in pairs(produced_wares_counters) do
--       print("wares counter: ", k,v)
--    end
--    for k,v in pairs(produced_wares_strings) do
--       print("ware strings: ", k,v)
--    end
   return deduplicated_programs, produced_wares_counters, produced_wares_strings
end
