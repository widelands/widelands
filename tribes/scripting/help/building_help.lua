-- TODO(GunChleoc): get resi_00.png from C++

include "tribes/scripting/help/format_help.lua"

-- RST
-- building_help.lua
-- ---------------

-- Functions used in the ingame building help windows for formatting the text and pictures.

--  =======================================================
--  *************** Basic helper functions ****************
--  =======================================================

-- RST
-- .. function text_line(t1, t2[, imgstr = nil])
--
--    Creates a line of h3 formatted text followed by normal text and an image.
--
--    :arg t1: text in h3 format.
--    :arg t2: text in p format.
--    :arg imgstr: image aligned right.
--    :returns: header followed by normal text and image.
--
function text_line(t1, t2, imgstr)
   if imgstr then
      return "<rt text-align=left image=" .. imgstr .. " image-align=right><p font-size=13 font-color=D1D1D1>" ..  t1 .. "</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
   else
      return "<rt text-align=left><p font-size=13 font-color=D1D1D1>" ..  t1 .. "</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
   end
end


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

   local string = "image=" .. images[1]
   for k,v in ipairs({table.unpack(images,2)}) do
      string = string .. ";pics/arrow-right.png;" .. v
   end

   return rt(string, text)
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
function dependencies_resi(resource, items, text)
   if not text then
      text = ""
   end
   local string = "image=tribes/immovables/" .. resource  .. "/idle_00.png"
   for k,v in ipairs({table.unpack(items)}) do
      string = string .. ";pics/arrow-right.png;" ..  v.icon_name
   end
   return rt(string, p(text))
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
      local images = food_images[1]
      for k,v in ipairs({table.unpack(food_images,2)}) do
         images = images .. ";" .. v
      end
      result = image_line(images, 1, p(text)) .. result
   end
   if (result ~= "") then
      result = rt(h3(_"Food:")) .. result
   end
   return result
end


-- RST
-- .. function:: dependencies_training_weapons(weapons)
--
--    Creates a dependencies line for any number of weapons.
--
--    :arg weapons: an array of weapon names
--    :returns: a list weapons images with the producing and receiving building
--
function dependencies_training_weapons(weapons)
   local result = "";
   local producers = {};
   for count, weaponname in pairs(weapons) do
      local weapon_description = wl.Game():get_ware_description(weaponname)
      for i, producer in ipairs(weapon_description.producers) do
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
      for weaponname, hasweapon in pairs(weaponnames) do
         if (hasweapon) then
            if(weaponsstring ~= "") then
               weaponsstring = weaponsstring .. ";"
            end
            local weapon_description = wl.Game():get_ware_description(weaponname)
            weaponsstring = weaponsstring .. weapon_description.icon_name
         end
      end
      building_count = building_count + 1;
      result = result ..
         dependencies_basic(
            {producer_description.icon_name, weaponsstring},
            rt(p(producer_description.descname))
      )
   end
   if (result ~= "") then
      result = rt(h3(_"Equipment:")) .. result
   end
   return result
end


--  =======================================================
--  ************* Main buildinghelp functions *************
--  =======================================================


-- RST
-- .. function building_help_general_string(tribe, building_description)
--
--    Creates the string for the general section in building help
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt of the formatted text
--
function building_help_general_string(tribe, building_description)
   -- TRANSLATORS: Heading for a flavour text in the building help.
   local result = rt(h2(_"Lore")) ..
      rt("image=" .. building_description.representative_image, p(building_helptext_lore()))
   if (building_helptext_lore_author() ~= "") then
      result = result .. rt("text-align=right",
                            p("font-size=10 font-style=italic",
                              building_helptext_lore_author()))
   end

   result = result .. rt(h2(_"General"))
   result = result .. rt(h3(_"Purpose:"))

-- TODO(GunChleoc) "carrier" for headquarters, "ship" for ports, "scout" for scouts_hut, "shipwright" for shipyard?
-- TODO(GunChleoc) use aihints for gamekeeper, forester?
   local representative_resource = nil
   if (building_description.type_name == "productionsite" or
       building_description.type_name == "trainingsite") then
      representative_resource = building_description.output_ware_types[1]
      if(not representative_resource) then
         representative_resource = building_description.output_worker_types[1]
      end
   elseif (building_description.type_name == "militarysite") then
      representative_resource = wl.Game():get_worker_description(tribe.soldier)
-- TODO(GunChleoc) need a bob_descr for the ship -> port and shipyard
-- TODO(GunChleoc) create descr objects for flag, portdock, ...
   elseif (building_description.is_port or building_description.name == "shipyard") then
      representative_resource = nil
   elseif (building_description.type_name == "warehouse") then
      representative_resource = wl.Game():get_ware_description("log")
   end

   if(representative_resource) then
      result = result .. image_line(representative_resource.icon_name, 1, p(building_helptext_purpose()))
   else
      result = result .. rt(p(building_helptext_purpose()))
   end

   if (building_helptext_note() ~= "") then
      result = result .. rt(h3(_"Note:")) .. rt(p(building_helptext_note()))
   end

   if(building_description.type_name == "productionsite") then
      if(building_description.workarea_radius and building_description.workarea_radius > 0) then
         result = result .. text_line(_"Work area radius:", building_description.workarea_radius)
      end

   elseif(building_description.type_name == "warehouse") then
      result = result .. rt(h3(_"Healing:")
         .. p(ngettext("Garrisoned soldiers heal %d health point per second", "Garrisoned soldiers heal %d health points per second", building_description.heal_per_second):bformat(building_description.heal_per_second)))
      result = result .. text_line(_"Conquer range:", building_description.conquers)

   elseif(building_description.type_name == "militarysite") then
      result = result .. rt(h3(_"Healing:")
         .. p(ngettext("Garrisoned soldiers heal %d health point per second", "Garrisoned soldiers heal %d health points per second", building_description.heal_per_second):bformat(building_description.heal_per_second)))
      result = result .. text_line(_"Capacity:", building_description.max_number_of_soldiers)
      result = result .. text_line(_"Conquer range:", building_description.conquers)

   elseif(building_description.type_name == "trainingsite") then
      result = result .. text_line(_"Capacity:", building_description.max_number_of_soldiers)
   end
   result = result .. text_line(_"Vision range:", building_description.vision_range)
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
      for j, producer in ipairs(ware_description.producers) do
         if (tribe:has_building(producer.name)) then
            result = result .. dependencies(
               {producer, ware_description},
               _"%1$s from: %2$s":bformat(ware_description.descname, producer.descname)
            )
         end
      end
   end
   if (hasinput) then
      -- TRANSLATORS: Heading in the building help for wares that a building accepts (e.g. wheat for a mill).
      result =  rt(h3(_"Incoming:")) .. result
   end

   if ((not hasinput) and building_description.output_ware_types[1]) then
      result = result .. rt(h3(_"Collects:"))
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
      result = result .. rt(h3(_"Mines:"))
      for i, ware_description in ipairs(building_description.output_ware_types) do

         -- Need to hack this, because resource != produced ware.
         local resi_name = ware_description.name
         if(resi_name == "iron_ore") then resi_name = "iron"
         elseif(resi_name == "granite") then resi_name = "stones"
         elseif(resi_name == "diamond") then resi_name = "stones"
         elseif(resi_name == "quartz") then resi_name = "stones"
         elseif(resi_name == "marble") then resi_name = "stones"
         elseif(resi_name == "gold_ore") then resi_name = "gold" end
         result = result .. dependencies_resi(
            "resi_"..resi_name.."2",
            {building_description, ware_description},
            ware_description.descname
         )
      end

   else
      if(building_description.output_ware_types[1] or building_description.output_worker_types[1]) then
         result = result .. rt(h3(_"Produces:"))
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
      for j, consumer in ipairs(ware_description.consumers) do
         if (tribe:has_building(consumer.name)) then
            outgoing = outgoing .. dependencies({ware_description, consumer}, consumer.descname)
         end
      end

      -- Soldiers aren't listed with the consumers. Get their buildcost wares and list the warehouses.
      local soldier = wl.Game():get_worker_description(tribe.soldier)
      for j, buildcost in ipairs(soldier.buildcost) do
         if (buildcost == ware) then
            for k, buildingname in ipairs(tribe.buildings) do
               local warehouse_description = wl.Game():get_building_description(buildingname)
               if (warehouse_description.type_name == "warehouse") then
                  outgoing = outgoing .. dependencies({ware, warehouse_description, soldier}, soldier.descname)
               end
            end
         end
      end
   end
   if (outgoing ~= "") then result = result .. rt(h3(_"Outgoing:")) .. outgoing end

   if (result == "") then result = rt(p(_"None")) end
   return rt(h2(_"Dependencies")) .. result
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
   if (building_description.max_hp and building_description.min_hp) then
      result = result .. rt(h2(_"Health Training"))
      result = result .. rt(p(_"Trains ‘Health’ from %1% up to %2%":
            bformat(building_description.min_hp, building_description.max_hp+1)))
      result = result .. rt(h3(_"Soldiers:"))
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/hp_level" .. building_description.min_hp .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/hp_level" .. (building_description.max_hp + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_hp)
      result = result .. dependencies_training_weapons(building_description.weapons_hp)
   end
   if (building_description.max_attack and building_description.min_attack) then
      result = result .. rt(h2(_"Attack Training"))
      -- TRANSLATORS: %1$s = Health, Evade, Attack or Defense. %2$s and %3$s are numbers.
      result = result .. rt(p(_"Trains ‘Attack’ from %1% up to %2%":
         bformat(building_description.min_attack, building_description.max_attack+1)))
      result = result .. rt(h3(_"Soldiers:")) ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/attack_level" .. building_description.min_attack .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/attack_level" .. (building_description.max_attack + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_attack)
      result = result .. dependencies_training_weapons(building_description.weapons_attack)
   end
   if (building_description.max_defense and building_description.min_defense) then
      result = result .. rt(h2(_"Defense Training"))
      result = result .. rt(p( _"Trains ‘Defense’ from %1% up to %2%":
            bformat(building_description.min_defense, building_description.max_defense+1)))
            result = result .. rt(h3(_"Soldiers:"))
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/defense_level" .. building_description.min_defense .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/defense_level" .. (building_description.max_defense + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_defense)
      result = result .. dependencies_training_weapons(building_description.weapons_defense)
   end
   if (building_description.max_evade and building_description.min_evade) then
      result = result .. rt(h2(_"Evade Training"))
      result = result .. rt(p( _"Trains ‘Evade’ from %1% up to %2%":
            bformat(building_description.min_evade, building_description.max_evade+1)))
      result = result .. rt(h3(_"Soldiers:"))
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/evade_level" .. building_description.min_evade .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/evade_level" .. (building_description.max_evade + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_evade)
      result = result .. dependencies_training_weapons(building_description.weapons_evade)
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
   local result = rt(h2(_"Building"))

   -- Space required
   if (building_description.is_mine) then
      result = result .. text_line(_"Space required:",_"Mine plot","pics/mine.png")
   elseif (building_description.is_port) then
      result = result .. text_line(_"Space required:",_"Port plot","pics/port.png")
   else
      if (building_description.size == 1) then
         result = result .. text_line(_"Space required:",_"Small plot","pics/small.png")
      elseif (building_description.size == 2) then
         result = result .. text_line(_"Space required:",_"Medium plot","pics/medium.png")
      elseif (building_description.size == 3) then
         result = result .. text_line(_"Space required:",_"Big plot","pics/big.png")
      else
         result = result .. p(_"Space required:" .. _"Unknown")
      end
   end

   -- Enhanced from
   if (building_description.buildable or building_description.enhanced) then

      if (building_description.buildable and building_description.enhanced) then
         result = result .. text_line(_"Note:",
            _"This building can either be built directly or obtained by enhancing another building.")
      end

      if (building_description.buildable) then
         -- Build cost
         if (building_description.buildable and building_description.enhanced) then
            result = result .. rt(h3(_"Direct build cost:"))
         else
            result = result .. rt(h3(_"Build cost:"))
         end
         for ware, amount in pairs(building_description.build_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end
      local former_building = nil
      if (building_description.enhanced) then
         former_building = building_description.enhanced_from
            if (building_description.buildable) then
               result = result .. text_line(_"Or enhanced from:", former_building.descname)
            else
               result = result .. text_line(_"Enhanced from:", former_building.descname)
            end

         for ware, amount in pairs(building_description.enhancement_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end

         -- Cumulative cost
         result = result .. rt(h3(_"Cumulative cost:"))
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
               for ware, amount in pairs(former_building.build_cost) do
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
            result = result .. rt(p(_"Unknown"))
         end

         -- Dismantle yields
         if (building_description.buildable) then
            result = result .. rt(h3(_"If built directly, dismantle yields:"))
            for ware, amount in pairs(building_description.returned_wares) do
               local ware_description = wl.Game():get_ware_description(ware)
               result = result .. help_ware_amount_line(ware_description, amount)
            end
            result = result .. rt(h3(_"If enhanced, dismantle yields:"))
         else
            -- TRANSLATORS: This is a heading for the resources that you will get back when you dismantle a building of this type. What dismantling will give you.
            result = result .. rt(h3(_"Dismantle yields:"))
         end
         local warescost = {}
         for ware, amount in pairs(building_description.returned_wares_enhanced) do
            if (warescost[ware]) then
               warescost[ware] = warescost[ware] + amount
            else
               warescost[ware] = amount
            end
         end
         for index, former in pairs(former_buildings) do
            former_building = wl.Game():get_building_description(former.name)
            if (former_building.buildable) then
               for ware, amount in pairs(former_building.returned_wares) do
                  if (warescost[ware]) then
                     warescost[ware] = warescost[ware] + amount
                  else
                     warescost[ware] = amount
                  end
               end
            elseif (former_building.enhanced) then
               for ware, amount in pairs(former_building.returned_wares_enhanced) do
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
            result = result .. rt(p(_"Unknown"))
         end
      -- Buildable
      else
         -- Dismantle yields
         result = result .. rt(h3(_"Dismantle yields:"))
         for ware, amount in pairs(building_description.returned_wares) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end

      -- Can be enhanced to
      if (building_description.enhancement) then
         result = result .. text_line(_"Can be enhanced to:", building_description.enhancement.descname)
         for ware, amount in pairs(building_description.enhancement.enhancement_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end
   end
   return result
end



-- RST
-- .. function building_help_crew_string(tribe, building_description)
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

      result = result .. rt(h2(_"Workers")) .. rt(h3(_"Crew required:"))

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

      result = result .. help_tool_string(tribe, toolnames, number_of_workers)

      if(becomes_description) then

         result = result .. rt(h3(_"Experience levels:"))
         -- TRANSLATORS: EP = Experience Points
         local exp_string = _"%s to %s (%s EP)":format(
               worker_description.descname,
               becomes_description.descname,
               worker_description.needed_experience
            )

         worker_description = becomes_description
         becomes_description = worker_description.becomes
         if(becomes_description) then
            exp_string = exp_string .. "<br>" .. _"%s to %s (%s EP)":format(
                  worker_description.descname,
                  becomes_description.descname,
                  worker_description.needed_experience
               )
         end
         result = result ..  rt("text-align=right", p(exp_string))
      end
   end

   return result
end


-- RST
-- .. building_help_production_section()
--
--    Displays the production/performance section with a headline
--
--    :returns: rt for the production section
--
function building_help_production_section()
   if (building_helptext_performance() ~= "") then
      return rt(h2(_"Production")) ..
        text_line(_"Performance:", building_helptext_performance())
   else
      return ""
   end
end


-- RST
-- .. function building_help(tribe, building_description)
--
--    Main function to create a building help string.
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`LuaBuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt of the formatted text
--
function building_help(tribe, building_description)
   include(building_description.helptext_script)

   if (building_description.type_name == "productionsite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_dependencies_production(tribe, building_description) ..
         building_help_crew_string(tribe, building_description) ..
         building_help_building_section(building_description) ..
         building_help_production_section()
   elseif (building_description.type_name == "militarysite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_building_section(building_description)
   elseif (building_description.type_name == "warehouse") then
      if (building_description.is_port) then
         return building_help_general_string(tribe, building_description) ..
            -- TODO(GunChleoc) expedition costs here?
            building_help_building_section(building_description) ..
            building_help_production_section()
      else
         return building_help_general_string(tribe, building_description) ..
            building_help_building_section(building_description)
      end
   elseif (building_description.type_name == "trainingsite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_dependencies_training(tribe, building_description) ..
         building_help_crew_string(tribe, building_description) ..
         building_help_building_section(building_description) ..building_help_production_section()
   elseif (building_description.type_name == "constructionsite" or
            building_description.type_name == "dismantlesite") then
            -- TODO(GunChleoc) Get them a crew string for the builder
      return building_help_general_string(tribe, building_description)
   else
      return ""
   end
end

-- The main function call
return {
   func = function(tribename, building)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      -- We need to get the building description again, because it will
      -- give us a cast to the appropriate subclass.
      local building_description = wl.Game():get_building_description(building.name)
      return building_help(tribe, building_description)
   end
}
