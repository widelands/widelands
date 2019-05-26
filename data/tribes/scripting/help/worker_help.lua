-- RST
-- worker_help.lua
-- ---------------
--
-- This script returns a formatted entry for the ingame worker help.
-- Pass the internal tribe name and worker name to the coroutine to select the
-- worker type.

include "tribes/scripting/help/format_help.lua"

--  =======================================================
--  ************* Main worker help functions *************
--  =======================================================

-- RST
-- .. function:: worker_help_producers_string(worker_description)
--
--    Displays the buildings that can produce the worker
--
--    :arg tribe: the worker's tribe from C++.
--    :arg worker_description: the worker_description from C++.
--    :returns: Info about buildings that produce this worker
--
function worker_help_producers_string(tribe, worker_description)
   local result = ""
   for i, building in ipairs(tribe.buildings) do
      if (building.type_name == "productionsite") then
         local recruits_this = false;
         for j, output in ipairs(building.output_worker_types) do
            if (output.name == worker_description.name) then
               recruits_this = true;
               break;
            end
         end

         if (recruits_this) then
            -- TRANSLATORS: Worker Encyclopedia: A building recruiting a worker
            result = result .. h2(_"Producer")
            result = result .. dependencies({building, worker_description}, building.descname)

            -- Find out which programs in the building recruit this worker if any
            local producing_programs = {}
            for j, program_name in ipairs(building.production_programs) do
               for worker, amount in pairs(building:recruited_workers(program_name)) do
                  if (worker_description.name == worker) then
                     table.insert(producing_programs, program_name)
                  end
               end
            end

            -- Now collect all workers recruited by the filtered programs
            local recruited_workers_strings = {}
            local recruited_workers_counters = {}
            for j, program_name in ipairs(producing_programs) do
               local recruited_workers_amount = {}
               recruited_workers_counters[program_name] = 0
               for worker, amount in pairs(building:recruited_workers(program_name)) do
                  if (recruited_workers_amount[worker] == nil) then
                     recruited_workers_amount[worker] = 0
                  end
                  recruited_workers_amount[worker] = recruited_workers_amount[worker] + amount
                  recruited_workers_counters[program_name] = recruited_workers_counters[program_name] + amount
               end
               local produced_wares_string = ""
               for ware, amount in pairs(recruited_workers_amount) do
               local ware_descr = wl.Game():get_worker_description(ware)
                  produced_wares_string = produced_wares_string
                     .. help_ware_amount_line(ware_descr, amount)
               end
               recruited_workers_strings[program_name] = produced_wares_string
            end

            -- Now collect the consumed wares for each filtered program and print the program info
            for j, program_name in ipairs(producing_programs) do
               result = result .. help_consumed_wares_workers(tribe, building, program_name)
               if (recruited_workers_counters[program_name] > 0) then
                  if (recruited_workers_counters[program_name] == 1) then
                     -- TRANSLATORS: Worker Encyclopedia: 1 worker recruited by a productionsite
                     result = result .. h3(_"Worker recruited:")
                  else
                     -- TRANSLATORS: Worker Encyclopedia: More than 1 worker recruited by a productionsite
                     result = result .. h3(_"Workers recruited:")
                  end
                  result = result .. recruited_workers_strings[program_name]
               end
            end
         end
      end
   end
   return result
end


-- RST
-- .. function:: worker_help_employers_string(worker_description)
--
--    Displays the buildings where the worker can work
--
--    :arg worker_description: the worker_description from C++.
--    :returns: Info about buildings that use this worker
--
function worker_help_employers_string(worker_description)
   local result = ""

   if (#worker_description.employers > 0) then
      local normal = {}
      local additional = {}

      if (#worker_description.employers == 1) then
      -- TRANSLATORS: Worker Encyclopedia: Heading for 1 building where a worker can work
      -- TRANSLATORS: You can also translate this as 'workplace'
         result = result .. h2(pgettext("workerhelp_one_building", "Works at"))
      else
      -- TRANSLATORS: Worker Encyclopedia: A list of more than 1 building where a worker can work
      -- TRANSLATORS: You can also translate this as 'workplaces'
         result = result .. h2(pgettext("workerhelp_multiple_buildings", "Works at"))
      end
      for i, building in ipairs(worker_description.employers) do
         result = result .. dependencies({worker_description, building}, building.descname)
         normal[building.descname] = true
      end
      building = worker_description.employers[1]
         if #building.working_positions > 1 and worker_description.name ~= building.working_positions[1].name then
            for i, build in ipairs(building.working_positions[1].employers) do
               if not normal[build.descname] then
                  table.insert(additional, build)
               end
            end
            if #additional == 1 then
               -- Translators: Worker Encyclopedia: Heading above a list 1 building where a worker may work instead of a less experienced worker
               -- TRANSLATORS: You can also translate this as 'additional workplace'
               result = result .. h2(pgettext("workerhelp_one_building", "Can also work at"))
            else
               -- Translators: Worker Encyclopedia: Heading above a list of buildings where a worker may work instead of a less experienced worker
               -- TRANSLATORS: You can also translate this as 'additional workplaces'
               result = result .. h2(pgettext("workerhelp_multiple_buildings", "Can also work at"))
            end
            for i, build in ipairs(additional) do
               result = result .. dependencies({worker_description, build}, build.descname)
            end
         end
   end
   return result
end


-- RST
-- .. function:: worker_help_string(worker_description)
--
--    Displays the worker with a helptext, an image and the tool used
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe
--                that we are displaying this help for.

--    :arg worker_description: the worker_description from C++.
--
--    :returns: Help string for the worker
--
function worker_help_string(tribe, worker_description)
   include(worker_description.helptext_script)

   local result = h2(_"Purpose") ..
      li_image(worker_description.icon_name, worker_helptext())

   if (worker_description.buildable) then
      -- Get the tools for the workers.
      local toolnames = {}
      for j, buildcost in ipairs(worker_description.buildcost) do
         if (buildcost ~= nil and tribe:has_ware(buildcost)) then
            toolnames[#toolnames + 1] = buildcost
         end
      end

      if (#toolnames > 0) then
         local tool_string = help_tool_string(tribe, toolnames, 1)
         -- TRANSLATORS: Tribal Encyclopedia: Heading for which tool a worker uses
         result = result .. h2(_"Worker uses") .. tool_string
      end
   else
      result = result .. worker_help_producers_string(tribe, worker_description)
   end

   result = result .. worker_help_employers_string(worker_description)

   -- TODO(GunChleoc): Add "enhanced from" info in one_tribe branch
   local becomes_description = worker_description.becomes
   if (becomes_description) then

      result = result .. help_worker_experience(worker_description, becomes_description)
   end
   -- Soldier properties
   if (worker_description.type_name == "soldier") then
      -- TRANSLATORS: Soldier levels
      result = result .. h2(_"Levels")

      result = result .. h3(_"Health")
      result = result ..
         li(
            -- TRANSLATORS: Soldier health / defense / evade points. A 5 digit number.
            (_"Starts at %1% points."):bformat(worker_description.base_health)) ..
         li(
            -- TRANSLATORS: Soldier health / attack defense / evade points
            ngettext("Increased by %1% point for each level.", "Increased by %1% points for each level.", worker_description.health_incr_per_level):bformat(worker_description.health_incr_per_level)) ..
         li(
            -- TRANSLATORS: Soldier health / attack defense / evade level
            ngettext("The maximum level is %1%.", "The maximum level is %1%.", worker_description.max_health_level):bformat(worker_description.max_health_level))

      result = result .. h3(_"Attack")
      result = result ..
      -- TRANSLATORS: Points are 4 digit numbers.
         li(_"A random value between %1% and %2% points is added to each attack."):bformat(worker_description.base_min_attack, worker_description.base_max_attack) ..

         li(
            ngettext("Increased by %1% point for each level.", "Increased by %1% points for each level.", worker_description.attack_incr_per_level):bformat(worker_description.attack_incr_per_level)) ..
         li(
            ngettext("The maximum level is %1%.", "The maximum level is %1%.", worker_description.max_attack_level):bformat(worker_description.max_attack_level))

      result = result .. h3(_"Defense")
      if (worker_description.max_defense_level > 0) then
         result = result ..
            li(
               (_"Starts at %d%%."):bformat(worker_description.base_defense)) ..
            li(
               (_"Increased by %d%% for each level."):bformat(worker_description.defense_incr_per_level)) ..
            li(
               ngettext("The maximum level is %1%.", "The maximum level is %1%.", worker_description.max_defense_level):bformat(worker_description.max_defense_level))
      else
         result = result ..
            li(
               (_"Starts at %d%%."):bformat(worker_description.base_defense)) ..
            li(_"This soldier cannot be trained in defense.")
      end

      result = result .. h3(_"Evade")
      result = result ..
         li(
            (_"Starts at %d%%."):bformat(worker_description.base_evade)) ..
         li(
            (_"Increased by %d%% for each level."):bformat(worker_description.evade_incr_per_level)) ..
         li(
            ngettext("The maximum level is %1%.", "The maximum level is %1%.", worker_description.max_evade_level):bformat(worker_description.max_evade_level))
   end
   return result
end


return {
   func = function(tribename, workername)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      local worker_description = wl.Game():get_worker_description(workername)
      return {
         title = worker_description.descname,
         text = worker_help_string(tribe, worker_description)
      }
   end
}
