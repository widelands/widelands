include "tribes/scripting/help/format_help.lua"
include "tribes/scripting/help/calculations.lua"


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
-- .. function:: item_image(mapobject)
--
--    Creates Richtext image code for the given map object.
--
--    :arg mapobject: the mapobject to represent
--    :returns: richtext code for displaying the mapobject as an image, preferable using its menu icon
--
function item_image(mapobject)
   local icon = mapobject.icon_name
   if icon ~= nil and icon ~= "" then
      return img(icon)
   end
   if mapobject.max_amount ~= nil then
      -- We have e.g. an undetectable resource and are thus without any resource indicator to show
      return img(mapobject:editor_image(1))
   end
   print("WARNING: help item without icon_name: " .. mapobject.name)
   return img_object(mapobject.name, "width=10")
end


-- RST
-- .. function:: find_resource_indicator(tribe, resource)
--
--    Returns the biggest resource indicator available.
--
--    :arg tribe: the tribe to use for getting a tribe-specific indicator
--    :arg resource: a map resource description
--    :returns: a resource indicator immovable description
--
function find_resource_indicator(tribe, resource)
   local resi = nil
   local am = 0
   local resource_indicators = tribe.resource_indicators[resource.name]
   if resource_indicators ~= nil then
      for amount, name in pairs(tribe.resource_indicators[resource.name]) do
         if amount > am then
            resi = name
            am = amount
         end
      end
   end
   if resi ~= nil then
      return wl.Game():get_immovable_description(resi)
   end
   return resource
end


-- RST
-- .. function:: find_created_collected_matches(creator, collector)
--
--    Returns a table of map objects that link the two given production sites
--
--    :arg creator: a productionsite description that creates bobs, immovables and/or resources
--    :arg collector: a productionsite description that collects bobs, immovables and/or resources
--    :returns: a list of all bob, immovable and resource descriptions that matches both
--
function find_created_collected_matches(creator, collector)
   local matching_items = {}
   for i, bob1 in ipairs(collector.collected_bobs) do
      for j, bob2 in ipairs(creator.created_bobs) do
         if bob1.name == bob2.name then
            table.insert(matching_items, bob1)
         end
      end
   end
   for i, immovable1 in ipairs(collector.collected_immovables) do
      for i, immovable2 in ipairs(creator.created_immovables) do
         if immovable1.name == immovable2.name then
            table.insert(matching_items, immovable1)
         end
      end
   end
   for i, resource1 in ipairs(collector.collected_resources) do
      for i, resource2 in ipairs(creator.created_resources) do
         if resource1.resource.name == resource2.name then
            table.insert(matching_items, resource1.resource)
         end
      end
   end
   return matching_items
end

--  =======================================================
--  *************** Dependencies functions ****************
--  =======================================================

-- RST
-- .. function:: dependencies_collects(tribe, building_description)
--
--    Assemble dependency help for a building that collects bobs, immovables and/or resources
--
--    :arg tribe: the tribe for which to display this help
--    :arg building_description: a production site that collects bobs, immovables and/or resources from the map
--    :returns: a richtext-formatted ``p()``
--
function dependencies_collects(tribe, building_description)
   local supported = building_description.supported_by_productionsites
   if #supported > 0 then
      local result = ""
      for i,productionsite in ipairs(supported) do
         local row = item_image(productionsite) .. img("images/richtext/arrow-right.png")
         for k,mapobject in ipairs(find_created_collected_matches(productionsite, building_description)) do
            row = row .. item_image(mapobject)
         end
         row = row .. img("images/richtext/arrow-right.png") .. img(building_description.icon_name) .. " " .. linkify_encyclopedia_object(productionsite)
         result = result .. p(row)
      end
      return result
   end
   -- Not supported by other productionsites
   local result = ""
   local collected_items = {}
   for i, bob in ipairs(building_description.collected_bobs) do
      table.insert(collected_items, {bob, true})
      result = result .. item_image(bob)
   end
   for i, immovable in ipairs(building_description.collected_immovables) do
      table.insert(collected_items, {immovable, true})
      result = result .. item_image(immovable)
   end
   for i, resource in ipairs(building_description.collected_resources) do
      table.insert(collected_items, {resource.resource, false})
      result = result .. item_image(find_resource_indicator(tribe, resource.resource))
   end
   result = result .. img("images/richtext/arrow-right.png") .. img(building_description.icon_name)
   for k,mapobject in ipairs(collected_items) do
      if k > 1 then result = result .. " • " end
      result = result .. (mapobject[2] and linkify_encyclopedia_object(mapobject[1]) or mapobject[1].descname)
   end
   return p(result)
end


-- RST
-- .. function:: dependencies_creates(tribe, building_description)
--
--    Assemble dependency help for a building that places bobs, immovables and/or resources on the map
--
--    :arg tribe: the tribe for which to display this help
--    :arg building_description: a production site that creates bobs, immovables and/or resources
--    :returns: a richtext-formatted ``p()``
--
function dependencies_creates(tribe, building_description)
   local supported = building_description.supported_productionsites
   if #supported > 0 then
      local result = ""
      for i,productionsite in ipairs(supported) do
         local row = img(building_description.icon_name) .. img("images/richtext/arrow-right.png")
         for k,mapobject in ipairs(find_created_collected_matches(building_description, productionsite)) do
            row = row .. item_image(mapobject)
         end
         row = row .. img("images/richtext/arrow-right.png") .. item_image(productionsite) .. " " .. linkify_encyclopedia_object(productionsite)
         result = result .. p(row)
      end
      return result
   end
   -- No other productionsites supported
   local result = img(building_description.icon_name) .. img("images/richtext/arrow-right.png")
   local created_items = {}
   for i, bob in ipairs(building_description.created_bobs) do
      table.insert(created_items, bob)
      result = result .. item_image(bob)
   end
   for i, immovable in ipairs(building_description.created_immovables) do
      table.insert(created_items, immovable)
      result = result .. item_image(immovable)
   end
   for i, resource in ipairs(building_description.created_resources) do
      table.insert(created_items, resource)
      result = result .. item_image(find_resource_indicator(tribe, resource))
   end
   result = result .. " " .. linkify_encyclopedia_object(created_items[1])
   for k,mapobject in ipairs({table.unpack(created_items,2)}) do
      result = result .. " • " .. linkify_encyclopedia_object(mapobject)
   end
   return p(result)
end


-- RST
-- .. function:: dependencies_training_food(foods)
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
         food_warenames[countfood] = linkify_encyclopedia_object(ware_description)
         food_images[countfood] = ware_description.icon_name
      end
      local text = localize_list(food_warenames, "or")
      if (countlist > 1) then
         text = _("%s and"):bformat(text)
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
      result = h3(_("Food:")) .. result
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
            weaponsstring .. " " .. linkify_encyclopedia_object(producer_description)
         )
   end
   if (result ~= "") then
      result = h3(_("Equipment:")) .. result
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
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt of the formatted text
--
function building_help_general_string(tribe, building_description)
   local helptexts = building_description:helptexts(tribe.name)
   -- TRANSLATORS: Lore helptext for a building - it hasn't been written yet.
   local lore_text = _("Text needed")
   if helptexts["lore"] ~= nil then
      lore_text = helptexts["lore"]
   end
   -- TRANSLATORS: Heading for a flavour text in the building help.
   local result = h2(_("Lore")) ..
      li_object(building_description.name, lore_text)

   local lore_author = ""
   if helptexts["lore_author"] ~= nil then
      lore_author = helptexts["lore_author"]
   end
   result = result .. div("width=100%", p_font("align=right", "size=10 italic=1", lore_author .. vspace(3)))

   result = result .. h2(_("General"))
   result = result .. h3(_("Purpose:"))

   local representative_resource = nil
   if (building_description.type_name == "productionsite") then
      representative_resource = building_description.output_ware_types[1]
      if not representative_resource then
         representative_resource = building_description.output_worker_types[1]
      end
      if not representative_resource then
          for i, bob in ipairs(building_description.created_bobs) do
            representative_resource = bob
            break
         end
      end
      if not representative_resource then
          for i, immovable in ipairs(building_description.created_immovables) do
            representative_resource = immovable
            break
         end
      end
      if not representative_resource then
          for i, resource in ipairs(building_description.created_resources) do
            representative_resource = find_resource_indicator(tribe, resource)
            if representative_resource.max_amount ~= nil then
               -- We have e.g. an undetectable resource and thus without any resource indicator to show
               representative_resource = representative_resource:editor_image(1)
            end
            break
         end
      end
      if not representative_resource then
         -- currently only scouts
         representative_resource = building_description.working_positions[1]
      end
   elseif (building_description.type_name == "militarysite" or
           building_description.type_name == "trainingsite") then
      representative_resource = wl.Game():get_worker_description(tribe.soldier)
   elseif (building_description.is_port) then
      representative_resource = wl.Game():get_ship_description(tribe.name .. "_ship")
   elseif (building_description.type_name == "warehouse") then
      if (building_description.conquers < 1) then
         representative_resource = wl.Game():get_ware_description("log")
      else
         representative_resource = wl.Game():get_worker_description(tribe.name .. "_carrier")
      end
   elseif (building_description.type_name == "constructionsite") or (building_description.type_name == "dismantlesite") then
      representative_resource = wl.Game():get_worker_description(tribe.name .. "_builder")
   end

   -- TRANSLATORS: Purpose helptext for a building - it hasn't been written yet.
   if helptexts["purpose"] ~= nil then
      if representative_resource ~= nil and representative_resource ~= "" then
         if representative_resource.icon_name ~= nil and representative_resource.icon_name ~= "" then
            result = result .. li_image(representative_resource.icon_name, helptexts["purpose"])
         elseif representative_resource.name ~= nil and representative_resource.name ~= ""  then
            result = result .. li_object(representative_resource.name, helptexts["purpose"])
         else
            result = result .. li_image(representative_resource, helptexts["purpose"])
         end
      end
   else
      result = result .. p(_("Text needed"))
   end

   local note = ""
   -- Add mining resource percentage and chance to note
   if building_description.is_mine then
      local max_percent = 0
      local depletion_chance = 0
      local collected_resources = building_description.collected_resources
      local number_of_resources = #collected_resources
      if number_of_resources > 0 then
         for i, resource in ipairs(collected_resources) do
            max_percent = max_percent + resource.yield
            depletion_chance = depletion_chance + resource.when_empty
         end
         max_percent = max_percent / number_of_resources
         depletion_chance = depletion_chance / number_of_resources

         if max_percent < 99.9 then
            -- TRANSLATORS: Note helptext for a mine.
            note = note .. p(_("This mine will exploit %1%%% of its resources. From there on out, it will only have a %2%%% chance of finding any more of them."):bformat(max_percent, depletion_chance))
         else
            -- TRANSLATORS: Note helptext for a mine.
            note = note .. p(_("This mine will exploit all of its resources down to the deepest level. But even after having done so, it will still have a %1%%% chance of finding some more of them."):bformat(depletion_chance))
         end
      end
   end
   if helptexts["note"] ~= nil then
      note = note .. p(helptexts["note"])
   end
   if note ~= "" then
      result = result .. h3(_("Note:")) .. note
   end

   if(building_description.type_name == "productionsite") then
      if(building_description.workarea_radius and building_description.workarea_radius > 0) then
         result = result .. inline_header(_("Work area radius:"), building_description.workarea_radius)
      end

   elseif(building_description.type_name == "warehouse") then
      result = result .. inline_header(_("Healing:"),
         ngettext("Garrisoned soldiers heal %d health point per second.", "Garrisoned soldiers heal %d health points per second.", building_description.heal_per_second):bformat(building_description.heal_per_second))
      result = result .. inline_header(_("Conquer range:"), building_description.conquers)

   elseif(building_description.type_name == "militarysite") then
      result = result .. h3(_("Healing:"))
         .. p(ngettext("Garrisoned soldiers heal %d health point per second.", "Garrisoned soldiers heal %d health points per second.", building_description.heal_per_second):bformat(building_description.heal_per_second))
      result = result .. inline_header(_("Capacity:"), building_description.max_number_of_soldiers)
      result = result .. inline_header(_("Conquer range:"), building_description.conquers)
      result = result .. inline_header(_("Vision range:"), building_description.vision_range)

   elseif(building_description.type_name == "trainingsite") then
      result = result .. inline_header(_("Capacity:"), building_description.max_number_of_soldiers)
   end
   return result
end


-- RST
-- .. function:: building_help_dependencies_production(tribe, building_description)
--
--    The input and output wares of a productionsite
--
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_dependencies_production(tribe, building_description)
   local result = ""

   -- Providers
   local hasinput = false
   local inputs = ""
   for i, ware_description in ipairs(building_description.inputs) do
      hasinput = true
      for j, producer in ipairs(ware_description:producers(tribe.name)) do
         inputs = inputs .. dependencies(
            {producer, ware_description},
            _("%1$s from: %2$s"):bformat(linkify_encyclopedia_object(ware_description), linkify_encyclopedia_object(producer))
         )
      end
   end
   if (hasinput) then
      -- TRANSLATORS: Heading in the building help for wares that a building accepts (e.g. wheat for a mill).
      result = h3(_("Incoming:")) .. inputs
   end

   -- Collected items
   if #building_description.collected_immovables > 0 or
      #building_description.collected_resources > 0 or
      #building_description.collected_bobs > 0 then
      if (building_description.is_mine) then
         -- TRANSLATORS: This is a verb (The miner mines)
         result = result .. h3(_("Mines:"))
      else
         result = result .. h3(_("Collects:"))
      end
      result = result .. dependencies_collects(tribe, building_description)
   end

   -- Created items
   if #building_description.created_immovables > 0 or
      #building_description.created_resources > 0 or
      #building_description.created_bobs > 0 then
      result = result .. h3(_("Creates:")) .. dependencies_creates(tribe, building_description)
   end

   local outgoing = ""
   -- Produced items
   if (building_description.output_ware_types[1] or building_description.output_worker_types[1]) then
      for i, worker_description in ipairs(building_description.output_worker_types) do
         outgoing = outgoing ..
            dependencies({building_description, worker_description}, linkify_encyclopedia_object(worker_description))
      end
   end
   -- Consumers
   for i, ware_description in ipairs(building_description.output_ware_types) do

      -- Constructionsite isn't listed with the consumers, so we need a special check
      if (ware_description:is_construction_material(tribe.name)) then
         local constructionsite_description =
            wl.Game():get_building_description("constructionsite")
         outgoing = outgoing .. dependencies({ware_description, constructionsite_description},
                                              linkify_encyclopedia_object(constructionsite_description))
      end

      -- Normal buildings
      for j, consumer in ipairs(ware_description:consumers(tribe.name)) do
         if (tribe:has_building(consumer.name)) then
            outgoing = outgoing .. dependencies({ware_description, consumer}, linkify_encyclopedia_object(consumer))
         end
      end

      -- Soldiers aren't listed with the consumers. Get their buildcost wares and list the warehouses.
      local soldier = wl.Game():get_worker_description(tribe.soldier)
      for j, buildcost in ipairs(soldier.buildcost) do
         if (buildcost == ware) then
            for k, building in ipairs(tribe.buildings) do
               if (building.type_name == "warehouse") then
                  outgoing = outgoing .. dependencies({ware, building, soldier}, linkify_encyclopedia_object(soldier))
               end
            end
         end
      end
   end
   if (outgoing ~= "") then result = result .. h3(_("Outgoing:")) .. outgoing end
   if (result == "") then result = p(_("None")) end
   return h2(_("Dependencies")) .. result
end

-- RST
-- .. function:: building_help_dependencies_training(tribe, building_description)
--
--    Shows the production dependencies for a training site.
--
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt string with training dependencies information.
--
function building_help_dependencies_training(tribe, building_description)
   local result = ""
   if (building_description.max_health and building_description.min_health) then
      result = result .. h2(_("Health Training"))
      result = result .. p(_("Trains ‘Health’ from %1% up to %2%"):
            bformat(building_description.min_health, building_description.max_health+1))
      result = result .. h3(_("Soldiers:"))
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/health_level" .. building_description.min_health .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/health_level" .. (building_description.max_health + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_health)
      result = result .. dependencies_training_weapons(building_description.weapons_health, tribe.name)
   end
   if (building_description.max_attack and building_description.min_attack) then
      result = result .. h2(_("Attack Training"))
      -- TRANSLATORS: %1$s = Health, Evade, Attack or Defense. %2$s and %3$s are numbers.
      result = result .. p(_("Trains ‘Attack’ from %1% up to %2%"):
         bformat(building_description.min_attack, building_description.max_attack+1))
      result = result .. h3(_("Soldiers:")) ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/attack_level" .. building_description.min_attack .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/attack_level" .. (building_description.max_attack + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_attack)
      result = result .. dependencies_training_weapons(building_description.weapons_attack, tribe.name)
   end
   if (building_description.max_defense and building_description.min_defense) then
      result = result .. h2(_("Defense Training"))
      result = result .. p( _("Trains ‘Defense’ from %1% up to %2%"):
            bformat(building_description.min_defense, building_description.max_defense+1))
            result = result .. h3(_("Soldiers:"))
      result = result ..
         dependencies_basic({
            "tribes/workers/" .. tribe.name .. "/soldier/defense_level" .. building_description.min_defense .. ".png",
            building_description.icon_name,
            "tribes/workers/" .. tribe.name .. "/soldier/defense_level" .. (building_description.max_defense + 1) ..".png"})
      result = result .. dependencies_training_food(building_description.food_defense)
      result = result .. dependencies_training_weapons(building_description.weapons_defense, tribe.name)
   end
   if (building_description.max_evade and building_description.min_evade) then
      result = result .. h2(_("Evade Training"))
      result = result .. p( _("Trains ‘Evade’ from %1% up to %2%"):
            bformat(building_description.min_evade, building_description.max_evade+1))
      result = result .. h3(_("Soldiers:"))
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
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: an rt string describing the building section
--
function building_help_building_section(building_description)
   -- TRANSLATORS: This is the header for the "Building" section in the building help, containing size info, buildcost etc.
   local result = h2(_("Building requirements"))

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
         result = result .. inline_header(_("Note:"),
            _("This building can either be built directly or obtained by enhancing another building."))
      end

      if (building_description.buildable) then
         -- Build cost
         if (building_description.buildable and building_description.enhanced) then
            result = result .. h3(_("Direct build cost:"))
         else
            result = result .. h3(_("Build cost:"))
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
               result = result .. inline_header(_("Or enhanced from:"), linkify_encyclopedia_object(former_building))
            else
               result = result .. inline_header(_("Enhanced from:"), linkify_encyclopedia_object(former_building))
            end

         for ware, amount in pairs(building_description.enhancement_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end

         -- Cumulative cost
         result = result .. h3(_("Cumulative cost:"))
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
            result = result .. p(_("Unknown"))
         end

         -- Dismantle yields
         if (building_description.buildable) then
            result = result .. h3(_("If built directly, dismantle yields:"))
            for ware, amount in pairs(building_description.returns_on_dismantle) do
               local ware_description = wl.Game():get_ware_description(ware)
               result = result .. help_ware_amount_line(ware_description, amount)
            end
            result = result .. h3(_("If enhanced, dismantle yields:"))
         else
            -- TRANSLATORS: This is a heading for the resources that you will get back when you dismantle a building of this type. What dismantling will give you.
            result = result .. h3(_("Dismantle yields:"))
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
            result = result .. p(_("Unknown"))
         end
      -- Buildable
      else
         -- Dismantle yields
         result = result .. h3(_("Dismantle yields:"))
         for ware, amount in pairs(building_description.returns_on_dismantle) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end

      -- Can be enhanced to
      if (building_description.enhancement) then
         result = result .. inline_header(_("Can be enhanced to:"), linkify_encyclopedia_object(building_description.enhancement))
         for ware, amount in pairs(building_description.enhancement.enhancement_cost) do
            local ware_description = wl.Game():get_ware_description(ware)
            result = result .. help_ware_amount_line(ware_description, amount)
         end
      end
   end
   return result
end



-- RST
-- .. function:: building_help_crew_section(tribe, building_description)
--
--    Displays the building's workers with an image and the tool they use
--
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe
--                that we are displaying this help for.
--
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--
--    :returns: Workers/Crew section of the help file
--
function building_help_crew_section(tribe, building_description)
   local result = ""

   if(building_description.type_name == "productionsite" or building_description.type_name == "trainingsite") then

      result = result .. h2(_("Workers")) .. h3(_("Crew required:"))

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
               p(_("%s or better"):bformat(linkify_encyclopedia_object(worker_description))))
         else
            result = result .. image_line(worker_description.icon_name, 1,
               p(linkify_encyclopedia_object(worker_description)))
         end
      end

      if (number_of_workers > 0) then
         local tool_string = help_tool_string(tribe, toolnames, number_of_workers)
         if (tool_string ~= "") then
            if (number_of_workers == 1) then
               -- TRANSLATORS: Tribal Encyclopedia: Heading for which tool 1 worker uses
               result = result .. h3(_("Worker uses:"))
            else
               -- TRANSLATORS: Tribal Encyclopedia: Heading for which tool more than 1 worker uses
               result = result .. h3(_("Workers use:"))
            end
            result = result .. tool_string
         end
      end

      worker_description = building_description.working_positions[1]
      becomes_description = worker_description.becomes

      if (becomes_description) then
         result = result .. h3(_("Experience levels"))
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
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--
--    :returns: rt for the production section
--
function building_help_production_section(tribe, building_description)
   -- Produced items
   local result = h2(_("Production"))
   if (building_description.output_ware_types[1] or building_description.output_worker_types[1]) then
      local checked_programs ={}
      for i, ware_description in ipairs(building_description.output_ware_types) do
         programs, ware_counters, ware_strings = programs_wares_count(tribe, building_description, ware_description)
         -- check if the ware is collected (no producing program)
         if #programs == 0 then
            result = result .. h3(_("Ware produced:"))
            result = result .. help_ware_amount_line(ware_description, 1)
            break
         end
         for j, program in ipairs(programs) do
            if (ware_counters[program] > 0) and not checked_programs[program] then
               if (ware_counters[program] == 1) then
                  -- TRANSLATORS: Ware Encyclopedia: 1 ware produced by a productionsite
                  result = result .. h3(_("Ware produced:"))
               else
                  -- TRANSLATORS: Ware Encyclopedia: More than 1 ware produced by a productionsite
                  result = result .. h3(_("Wares produced:"))
               end
               result = result .. ware_strings[program]
               result = result .. help_consumed_wares_workers(tribe, building_description, program)
            end
         checked_programs[program] = true
         end
      end
      for i, worker_description in ipairs(building_description.output_worker_types) do
         programs, worker_counters, worker_strings = programs_workers_count(tribe, building_description, worker_description)
         for j, program in ipairs(programs) do
            if (worker_counters[program] > 0) and not checked_programs[program] then
               -- TRANSLATORS: Ware Encyclopedia: 1 special worker or soldier recruited by a productionsite
               result = result .. h3(_("Workers recruited:"))
               result = result .. worker_strings[program]
               result = result .. help_consumed_wares_workers(tribe, building_description, program)
            end
         checked_programs[program] = true
         end
      end
   end
   local helptexts = building_description:helptexts(tribe.name)
   -- TRANSLATORS: Performance helptext for a building - it hasn't been written yet.
   local performance = _("Calculation needed")
   if (helptexts["performance"] ~= nil) then
      performance = helptexts["performance"]
   end
   result = result .. inline_header(_("Performance:"), performance)
   return result
end


-- RST
-- .. function:: building_help(tribe, building_description)
--
--    Main function to create a building help string.
--
--    :arg tribe: The :class:`wl.map.TribeDescription` for the tribe that has this building.
--    :arg building_description: The :class:`wl.map.BuildingDescription` for the building
--                               that we are displaying this help for.
--    :returns: rt of the formatted text
--
function building_help(tribe, building_description)
   if (building_description.type_name == "productionsite") then
      return building_help_general_string(tribe, building_description) ..
         building_help_dependencies_production(tribe, building_description) ..
         building_help_crew_section(tribe, building_description) ..
         building_help_production_section(tribe, building_description) ..
         building_help_building_section(building_description)
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
         building_help_crew_section(tribe, building_description) ..
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
