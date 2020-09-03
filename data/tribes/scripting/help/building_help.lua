include "tribes/scripting/help/format_help.lua"

-- RST
-- building_help.lua
-- -----------------
--
-- This script returns a formatted entry for the ingame building help.
-- Pass the internal tribe name and building name to the coroutine to select the
-- building type.

--  =======================================================
--  ********** Helper functions for dependencies **********
--  =======================================================

-- RST
-- .. function:: dependencies_basic(images[, text = nil])
--
--    Creates a dependencies line of any length.
--
--    :arg images: images in the correct order from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies_basic(images, text)
   if not text then
      text = ""
   end

   local imgstring = img(images[1])
   for k,v in ipairs({table.unpack(images,2)}) do
      imgstring = imgstring .. img("images/richtext/arrow-right.png") .. img(v)
   end
   return p(imgstring .. text)
end


-- RST
-- .. function:: dependencies_resi(resource, items[, text = nil])
--
--    Creates a dependencies line of any length for resources (that don't have menu.png files).
--
--    :arg resource: name of the geological resource.
--    :arg items: ware/building descriptions in the correct order from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies_resi(tribename, resource, items, text)
   if not text then
      text = ""
   end
   local tribe_descr = wl.Game():get_tribe_description(tribename)
   local resi
   local am = 0
   for amount,name in pairs(tribe_descr.resource_indicators[resource]) do
      if amount > am then
         resi = name
         am = amount
      end
   end
   local items_with_resource = { wl.Game():get_immovable_description(resi).icon_name }
   for count, item in pairs(items) do
      table.insert(items_with_resource, item.icon_name)
   end
   return dependencies_basic(items_with_resource, text)
end


--  =======================================================
--  *************** Dependencies functions ****************
--  =======================================================

-- RST
-- .. function:: dependencies_training_food
--
--    Creates dependencies lines for food in training sites.
--
--    :arg foods: an array of arrays with food items. Outer array has "and" logic and
--        will appear from back to front, inner arrays have "or" logic
--    :returns: a list of food descriptions with images
--
function dependencies_training_food(foods)
   local result = ""
   for countlist, foodlist in pairs(foods) do
      local food_warenames = {}
      local food_images = {}
      for countfood, food in pairs(foodlist) do
         local ware_description = wl.Game():get_ware_description(food)
         food_warenames[countfood] = ware_description.descname
         food_images[countfood] = ware_description.icon_name
      end
      local text = localize_list(food_warenames, "or")
      if (countlist > 1) then
         text = _"%s and":bformat(text)
      end
      local images = img(food_images[1])
      for k,v in ipairs({table.unpack(food_images,2)}) do
         images = images .. img(v)
      end
      result =
         div("width=100%",
            div("width=50%", p(vspace(6) .. text .. space(6))) ..
            div("width=*", p("align=right", vspace(6) .. images .. vspace(12)))
         )
         .. result
   end
   if (result ~= "") then
      result = h3(_"Food:") .. result
   end
   return result
end


-- RST
-- .. function:: dependencies_training_weapons(weapons)
--
--    Creates a dependencies line for any number of weapons.
--
--    :arg weapons: an array of weapon names
--    :arg tribename: the name of the tribe for filtering the buildings
--    :returns: a list weapons images with the producing and receiving building
--
function dependencies_training_weapons(weapons, tribename)
   local result = "";
   local producers = {};
   for count, weaponname in pairs(weapons) do
      local weapon_description = wl.Game():get_ware_description(weaponname)
      for i, producer in ipairs(weapon_description:producers(tribename)) do
         if (producers[producer.name] == nil) then
            producers[producer.name] = {}
         end
         producers[producer.name][weaponname] = true;
      end
   end

   local building_count = 0;
   for producer, weaponnames in pairs(producers) do
      local producer_description = wl.Game():get_building_description(producer)
      local weaponsstring = ""
      local weaponslist = { producer_description.icon_name }
      for weaponname, hasweapon in pairs(weaponnames) do
         if (hasweapon) then
            local weapon_description = wl.Game():get_ware_description(weaponname)
            if (#weaponslist < 2) then
                  table.insert(weaponslist, weapon_description.icon_name)
               else
                  weaponsstring = weaponsstring .. img(weapon_description.icon_name)
            end
         end
      end
      building_count = building_count + 1;
      result = result ..
         dependencies_basic(
            weaponslist,
            weaponsstring .. " " .. producer_description.descname
         )
   end
   if (result ~= "") then
      result = h3(_"Equipment:") .. result
   end
   return result
end


--  =======================================================
--  ************* Main buildinghelp functions *************
--  =======================================================


-- RST
-- .. function:: building_help_general_string(tribe, building_description)
--
--    Creates the string for the general section in building help
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt of the formatted text
--
function building_help_general_string(tribe, building_description)
   local helptexts = building_description:helptexts(tribe.name)
   -- TRANSLATORS: Lore helptext for a building - it hasn't been written yet.
   local lore_text = _"Text needed"
   if helptexts["lore"] ~= nil then
      lore_text = helptexts["lore"]
   end
   -- TRANSLATORS: Heading for a flavour text in the building help.
   local result = h2(_"Lore") ..
      li_object(building_description.name, lore_text)

   local lore_author = ""
   if helptexts["lore_author"] ~= nil then
      lore_author = helptexts["lore_author"]
   end
   result = result .. div("width=100%", p_font("align=right", "size=10 italic=1", lore_author .. vspace(3)))

   result = result .. h2(_"General")
   result = result .. h3(_"Purpose:")

-- TODO(GunChleoc) "carrier" for headquarters, "ship" for ports, "scout" for scouts_hut, "shipwright" for shipyard?
-- TODO(GunChleoc) use aihints for gamekeeper, forester?
   local representative_resource = nil
   if (building_description.type_name == "productionsite") then
      representative_resource = building_description.output_ware_types[1]
      if(not representative_resource) then
         representative_resource = building_description.output_worker_types[1]
      end
   elseif (building_description.type_name == "militarysite" or
       building_description.type_name == "trainingsite") then
      representative_resource = wl.Game():get_worker_description(tribe.soldier)
-- TODO(GunChleoc) need a bob_descr for the ship -> port and shipyard
-- TODO(GunChleoc) create descr objects for flag, portdock, ...
   elseif (building_description.is_port or building_description.name == "shipyard") then
      representative_resource = nil
   elseif (building_description.type_name == "warehouse") then
      representative_resource = wl.Game():get_ware_description("log")
   end

   -- TRANSLATORS: Purpose helptext for a building - it hasn't been written yet.
   local purpose = _"Text needed"
   if helptexts["purpose"] ~= nil then
      purpose = helptexts["purpose"]
   end
   if representative_resource then
      result = result .. li_image(representative_resource.icon_name, purpose)
   else
      result = result .. p(purpose)
   end

   if helptexts["note"] ~= nil then
      result = result .. h3(_"Note:") .. p(helptexts["note"])
   end

   if(building_description.type_name == "productionsite") then
      if(building_description.workarea_radius and building_description.workarea_radius > 0) then
         result = result .. inline_header(_"Work area radius:", building_description.workarea_radius)
      end

   elseif(building_description.type_name == "warehouse") then
      result = result .. inline_header(_"Healing:",
         ngettext("Garrisoned soldiers heal %d health point per second.", "Garrisoned soldiers heal %d health points per second.", building_description.heal_per_second):bformat(building_description.heal_per_second))
      result = result .. inline_header(_"Conquer range:", building_description.conquers)

   elseif(building_description.type_name == "militarysite") then
      result = result .. h3(_"Healing:")
         .. p(ngettext("Garrisoned soldiers heal %d health point per second.", "Garrisoned soldiers heal %d health points per second.", building_description.heal_per_second):bformat(building_description.heal_per_second))
      result = result .. inline_header(_"Capacity:", building_description.max_number_of_soldiers)
      result = result .. inline_header(_"Conquer range:", building_description.conquers)

   elseif(building_description.type_name == "trainingsite") then
      result = result .. inline_header(_"Capacity:", building_description.max_number_of_soldiers)
   end
   result = result .. inline_header(_"Vision range:", building_description.vision_range)
   return result
end


-- RST
-- .. function:: building_help_dependencies_production(tribe, building_description)
--
--    The input and output wares of a productionsite
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_dependencies_production(tribe, building_description)
   local result = ""
   local hasinput = false

   for i, ware_description in ipairs(building_description.inputs) do
    hasinput = true
      for j, producer in ipairs(ware_description:producers(tribe.name)) do
         result = result .. dependencies(
            {producer, ware_description},
            _"%1$s from: %2$s":bformat(ware_description.descname, producer.descname)
         )
      end
   end
   if (hasinput) then
      -- TRANSLATORS: Heading in the building help for wares that a building accepts (e.g. wheat for a mill).
      result =  h3(_"Incoming:") .. result
   end

   if ((not hasinput) and building_description.output_ware_types[1]) then
      result = result .. h3(_"Collects:")
      for i, ware_description in ipairs(building_description.output_ware_types) do
         result = result ..
            dependencies({building_description, ware_description}, ware_description.descname)
      end
      for i, worker_description in ipairs(building_description.output_worker_types) do
         result = result ..
            dependencies({building_description, worker_description}, worker_description.descname)
      end

   elseif (building_description.is_mine) then
      -- TRANSLATORS: This is a verb (The miner mines)
      result = result .. h3(_"Mines:")
      for i, ware_description in ipairs(building_description.output_ware_types) do

         -- Need to hack this, because resource != produced ware.
         local resi_name = ware_description.name
         if(resi_name == "coal") then resi_name = "resource_coal"
         elseif(resi_name == "iron_ore") then resi_name = "resource_iron"
         elseif(resi_name == "granite") then resi_name = "resource_stones"
         elseif(resi_name == "diamond") then resi_name = "resource_stones"
         elseif(resi_name == "quartz") then resi_name = "resource_stones"
         elseif(resi_name == "marble") then resi_name = "resource_stones"
         elseif(resi_name == "gold_ore") then resi_name = "resource_gold"
         elseif(resi_name == "water") then resi_name = "water" end

         result = result .. dependencies_resi(tribe.name,
            resi_name,
            {building_description, ware_description},
            ware_description.descname
         )
      end

   else
      if(building_description.output_ware_types[1] or building_description.output_worker_types[1]) then
         result = result .. h3(_"Produces:")
      end
      for i, ware_description in ipairs(building_description.output_ware_types) do
         result = result ..
            dependencies({building_description, ware_description}, ware_description.descname)
      end
      for i, worker_description in ipairs(building_description.output_worker_types) do
         result = result ..
            dependencies({building_description, worker_description}, worker_description.descname)
      end
   end

   local outgoing = ""
   for i, ware_description in ipairs(building_description.output_ware_types) do

      -- Constructionsite isn't listed with the consumers, so we need a special check
      if (ware_description:is_construction_material(tribe.name)) then
         local constructionsite_description =
            wl.Game():get_building_description("constructionsite")
         outgoing = outgoing .. dependencies({ware_description, constructionsite_description},
                                              constructionsite_description.descname)
      end

      -- Normal buildings
      for j, consumer in ipairs(ware_description:consumers(tribe.name)) do
         if (tribe:has_building(consumer.name)) then
            outgoing = outgoing .. dependencies({ware_description, consumer}, consumer.descname)
         end
      end

      -- Soldiers aren't listed with the consumers. Get their buildcost wares and list the warehouses.
      local soldier = wl.Game():get_worker_description(tribe.soldier)
      for j, buildcost in ipairs(soldier.buildcost) do
         if (buildcost == ware) then
            for k, building in ipairs(tribe.buildings) do
               if (building.type_name == "warehouse") then
                  outgoing = outgoing .. dependencies({ware, building, soldier}, soldier.descname)
               end
            end
         end
      end
   end
   if (outgoing ~= "") then result = result .. h3(_"Outgoing:") .. outgoing end
   if (result == "") then result = p(_"None") end
   return h2(_"Dependencies") .. result
end

-- RST
-- .. function:: building_help_dependencies_training(tribe, building_description)
--
--    Shows the production dependencies for a training site.
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt string with training dependencies information.
--
function building_help_dependencies_training(tribe, building_description)
   local result = ""
   if (building_description.max_health and building_description.min_health) then
      result = result .. h2(_"Health Training")
      result = result .. p(_"Trains ‘Health’ from %1% up to %2%":
            bformat(building_description.min_health, building_description.max_health+1))
      result = result .. h3(_"Soldiers:")
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/health_level" .. building_description.min_health .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/health_level" .. (building_description.max_health + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_health)
      result = result .. dependencies_training_weapons(building_description.weapons_health, tribe.name)
   end
   if (building_description.max_attack and building_description.min_attack) then
      result = result .. h2(_"Attack Training")
      -- TRANSLATORS: %1$s = Health, Evade, Attack or Defense. %2$s and %3$s are numbers.
      result = result .. p(_"Trains ‘Attack’ from %1% up to %2%":
         bformat(building_description.min_attack, building_description.max_attack+1))
      result = result .. h3(_"Soldiers:") ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/attack_level" .. building_description.min_attack .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/attack_level" .. (building_description.max_attack + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_attack)
      result = result .. dependencies_training_weapons(building_description.weapons_attack, tribe.name)
   end
   if (building_description.max_defense and building_description.min_defense) then
      result = result .. h2(_"Defense Training")
      result = result .. p( _"Trains ‘Defense’ from %1% up to %2%":
            bformat(building_description.min_defense, building_description.max_defense+1))
            result = result .. h3(_"Soldiers:")
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/defense_level" .. building_description.min_defense .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/defense_level" .. (building_description.max_defense + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_defense)
      result = result .. dependencies_training_weapons(building_description.weapons_defense, tribe.name)
   end
   if (building_description.max_evade and building_description.min_evade) then
      result = result .. h2(_"Evade Training")
      result = result .. p( _"Trains ‘Evade’ from %1% up to %2%":
            bformat(building_description.min_evade, building_description.max_evade+1))
      result = result .. h3(_"Soldiers:")
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/evade_level" .. building_description.min_evade .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/evade_level" .. (building_description.max_evade + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_evade)
      result = result .. dependencies_training_weapons(building_description.weapons_evade, tribe.name)
   end
   return result
end


-- RST
--
-- .. function:: building_help_building_section(building_description)
--
--    Formats the "Building" section in the building help: Enhancing info, costs and space required
--
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: an rt string describing the building section
--
function building_help_building_section(building_description)
   -- TRANSLATORS: This is the header for the "Building" section in the building help, containing size info, buildcost etc.
   local result = h2(_"Building")

   -- Space required
   if (building_description.is_mine) then
      result = result .. plot_size_line("mine")
   elseif (building_description.is_port) then
      result = result .. plot_size_line("port")
   else
      result = result .. plot_size_line(building_description.size)
   end

   -- Enhanced from
   if (building_description.buildable or building_description.enhanced) then

      if (building_description.buildable and building_description.enhanced) then
         result = result .. inline_header(_"Note:",
            _"This building can either be built directly or obtained by enhancing another building.")
      end

      if (building_description.buildable) then
         -- Build cost
         if (building_description.buildable and building_description.enhanced) then
            result = result .. h3(_"Direct build cost:")
         else
            result = result .. h3(_"Build cost:")
         end
         for ware, amount in pairs(building_description.buildcost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end
      local former_building = nil
      if (building_description.enhanced) then
         former_building = building_description.enhanced_from
            if (building_description.buildable) then
               result = result .. inline_header(_"Or enhanced from:", former_building.descname)
            else
               result = result .. inline_header(_"Enhanced from:", former_building.descname)
            end

         for ware, amount in pairs(building_description.enhancement_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end

         -- Cumulative cost
         result = result .. h3(_"Cumulative cost:")
         local warescost = {}
         for ware, amount in pairs(building_description.enhancement_cost) do
            if (warescost[ware]) then
               warescost[ware] = warescost[ware] + amount
            else
               warescost[ware] = amount
            end
         end

         local former_buildings = {};
         former_building = building_description

         while former_building.enhanced do
            former_building = former_building.enhanced_from
            table.insert(former_buildings, former_building)
         end

         for index, former in pairs(former_buildings) do
            former_building = wl.Game():get_building_description(former.name)
            if (former_building.buildable) then
               for ware, amount in pairs(former_building.buildcost) do
                  if (warescost[ware]) then
                     warescost[ware] = warescost[ware] + amount
                  else
                     warescost[ware] = amount
                  end
               end
            elseif (former_building.enhanced) then
               for ware, amount in pairs(former_building.enhancement_cost) do
                  if (warescost[ware]) then
                     warescost[ware] = warescost[ware] + amount
                  else
                     warescost[ware] = amount
                  end
               end
            end
         end

         if (warescost ~= {}) then
            for ware, amount in pairs(warescost) do
               local ware_description = wl.Game():get_ware_description(ware)
               result = result .. help_ware_amount_line(ware_description, amount)
            end
         else
            result = result .. p(_"Unknown")
         end

         -- Dismantle yields
         if (building_description.buildable) then
            result = result .. h3(_"If built directly, dismantle yields:")
            for ware, amount in pairs(building_description.returns_on_dismantle) do
               local ware_description = wl.Game():get_ware_description(ware)
               result = result .. help_ware_amount_line(ware_description, amount)
            end
            result = result .. h3(_"If enhanced, dismantle yields:")
         else
            -- TRANSLATORS: This is a heading for the resources that you will get back when you dismantle a building of this type. What dismantling will give you.
            result = result .. h3(_"Dismantle yields:")
         end
         local warescost = {}
         for ware, amount in pairs(building_description.enhancement_returns_on_dismantle) do
            if (warescost[ware]) then
               warescost[ware] = warescost[ware] + amount
            else
               warescost[ware] = amount
            end
         end
         for index, former in pairs(former_buildings) do
            former_building = wl.Game():get_building_description(former.name)
            if (former_building.buildable) then
               for ware, amount in pairs(former_building.returns_on_dismantle) do
                  if (warescost[ware]) then
                     warescost[ware] = warescost[ware] + amount
                  else
                     warescost[ware] = amount
                  end
               end
            elseif (former_building.enhanced) then
               for ware, amount in pairs(former_building.enhancement_returns_on_dismantle) do
                  if (warescost[ware]) then
                     warescost[ware] = warescost[ware] + amount
                  else
                     warescost[ware] = amount
                  end
               end
            end
         end
         if (warescost ~= {}) then
            for ware, amount in pairs(warescost) do
               local ware_description = wl.Game():get_ware_description(ware)
               result = result .. help_ware_amount_line(ware_description, amount)
            end
         else
            result = result .. p(_"Unknown")
         end
      -- Buildable
      else
         -- Dismantle yields
         result = result .. h3(_"Dismantle yields:")
         for ware, amount in pairs(building_description.returns_on_dismantle) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end

      -- Can be enhanced to
      if (building_description.enhancement) then
         result = result .. inline_header(_"Can be enhanced to:", building_description.enhancement.descname)
         for ware, amount in pairs(building_description.enhancement.enhancement_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end
   end
   return result
end



-- RST
-- .. function:: building_help_crew_string(tribe, building_description)
--
--    Displays the building's workers with an image and the tool they use
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe
--                that we are displaying this help for.
--
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--
--    :returns: Workers/Crew section of the help file
--
function building_help_crew_string(tribe, building_description)
   local result = ""

   if(building_description.type_name == "productionsite" or building_description.type_name == "trainingsite") then

      result = result .. h2(_"Workers") .. h3(_"Crew required:")

      local worker_description = building_description.working_positions[1]
      local becomes_description = nil
      local number_of_workers = 0
      local toolnames = {}

      for i, worker_description in ipairs(building_description.working_positions) do

         -- Get the tools for the workers.
         for j, buildcost in ipairs(worker_description.buildcost) do
            if (buildcost ~= nil and tribe:has_ware(buildcost)) then
               toolnames[#toolnames + 1] = buildcost
            end
         end

         becomes_description = worker_description.becomes
         number_of_workers = number_of_workers + 1

         if(becomes_description) then
            result = result .. image_line(worker_description.icon_name, 1,
               -- TRANSLATORS: %s is a worker name, e.g. "Chief Miner" or "Brewer".
               p(_"%s or better":bformat(worker_description.descname)))
         else
            result = result .. image_line(worker_description.icon_name, 1,
               p(worker_description.descname))
         end
      end

      if (number_of_workers > 0) then
         local tool_string = help_tool_string(tribe, toolnames, number_of_workers)
         if (tool_string ~= "") then
            if (number_of_workers == 1) then
               -- TRANSLATORS: Tribal Encyclopedia: Heading for which tool 1 worker uses
               result = result .. h3(_"Worker uses:")
            else
               -- TRANSLATORS: Tribal Encyclopedia: Heading for which tool more than 1 worker uses
               result = result .. h3(_"Workers use:")
            end
            result = result .. tool_string
         end
      end

      worker_description = building_description.working_positions[1]
      becomes_description = worker_description.becomes

      if (becomes_description) then
         result = result .. help_worker_experience(worker_description, becomes_description)
      end
   end

   return result
end


-- RST
-- .. function:: building_help_production_section(tribe, building_description)
--
--    Displays the production/performance section with a headline
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--
--    :returns: rt for the production section
--
function building_help_production_section(tribe, building_description)
   local helptexts = building_description:helptexts(tribe.name)
   -- TRANSLATORS: Performance helptext for a building - it hasn't been written yet.
   local performance = _"Calculation needed"
   if (helptexts["performance"] ~= nil) then
      performance = helptexts["performance"]
   end
   return h2(_"Production") ..
     inline_header(_"Performance:", performance)
end


-- RST
-- .. function:: building_help(tribe, building_description)
--
--    Main function to create a building help string.
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt of the formatted text
--
function building_help(tribe, building_description)
   if (building_description.type_name == "productionsite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_dependencies_production(tribe, building_description) ..
         building_help_crew_string(tribe, building_description) ..
         building_help_building_section(building_description) ..
         building_help_production_section(tribe, building_description)
   elseif (building_description.type_name == "militarysite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_building_section(building_description)
   elseif (building_description.type_name == "warehouse") then
      if (building_description.is_port) then
         return building_help_general_string(tribe, building_description) ..
            -- TODO(GunChleoc) expedition costs here?
            building_help_building_section(building_description)
      else
         return building_help_general_string(tribe, building_description) ..
            building_help_building_section(building_description)
      end
   elseif (building_description.type_name == "trainingsite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_dependencies_training(tribe, building_description) ..
         building_help_crew_string(tribe, building_description) ..
         building_help_building_section(building_description) ..building_help_production_section(tribe, building_description)
   elseif (building_description.type_name == "constructionsite" or
            building_description.type_name == "dismantlesite") then
            -- TODO(GunChleoc) Get them a crew string for the builder
      return building_help_general_string(tribe, building_description)
   elseif (building_description.type_name == "market") then
      return building_help_general_string(tribe, building_description)
   else
      return ""
   end
end

-- The main function call
return {
   func = function(tribename, buildingname)
      push_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      -- We need to get the building description again, because it will
      -- give us a cast to the appropriate subclass.
      local building_description = wl.Game():get_building_description(buildingname)
      local r = {
         title = building_description.descname,
         text = building_help(tribe, building_description)
      }
      pop_textdomain()
      return r
   end
}
