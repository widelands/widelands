-- TODO Textdomain?
-- TODO: menu.png, resi_00.png etc. will need to be replaced by representative_image eventually

--  =======================================================
--  *************** Basic helper functions ****************
--  =======================================================

-- RST
-- .. function:: image_line(image, count[, text = nil])
--
--    Aligns the image to a row on the right side with text on the left.
--
--    :arg image: the picture to be aligned to a row.
--    :arg count: length of the picture row.
--    :arg text: if given the text aligned on the left side, formatted via
--       formatting.lua functions.
--    :returns: the text on the left and a picture row on the right.
--
function image_line(image, count, text)
	local imgs={}
	for i=1,count do
		imgs[#imgs + 1] = image
	end
	local imgstr = table.concat(imgs, ";")

	if text then
		return rt("image=" .. imgstr .. " image-align=right", "  " .. text)
	else
		return rt("image=" .. imgstr .. " image-align=right", "")
	end
end

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

-- Tabs für die Hilfe, weiß aber nicht, ob das wirklich so funktionieren kann,
-- oder wie ich das eigentlich anwenden muss… Oder was ich falsch mache…
function make_tabs_array(t1, t2)
  return { {
        text = t1,
        tab_picture = "pics/small.png", -- Graphic for the tab button
     },
     {
        text = t2,
        tab_picture = "pics/medium.png",
     }
  }
end

--  =======================================================
--  ********** Helper functions for dependencies **********
--  =======================================================

-- RST
-- format_help.lua
-- ---------------

-- Functions used in the ingame help windows for formatting the text and pictures.

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

	string = "image=" .. images[1]
	for k,v in ipairs({table.unpack(images,2)}) do
		string = string .. ";pics/arrow-right.png;" .. v
	end

	return rt(string, text)
end


-- RST
-- .. function:: dependencies(tribename, items[, text = nil])
--
--    Creates a dependencies line of any length.
--
--    :arg tribename: name of the tribe.
--    :arg items: ware and/or building names in the correct order from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies(tribename, items, text)
	if not text then
		text = ""
	end
	string = "image=tribes/" .. tribename .. "/" .. items[1]  .. "/menu.png"
	for k,v in ipairs({table.unpack(items,2)}) do
		string = string .. ";pics/arrow-right.png;" ..  "tribes/" .. tribename .. "/" .. v  .. "/menu.png"
	end
	return rt(string, p(text))
end


-- RST
-- .. function:: dependencies_resi(tribename, items[, text = nil])
--
--    Creates a dependencies line of any length for resources (that don't have menu.png files).
--
--    :arg tribename: name of the tribe.
--    :arg items: resource names in the correct order from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies_resi(tribename, items, text)
	if not text then
		text = ""
	end
	string = "image=tribes/" .. tribename .. "/" .. items[1]  .. "/resi_00.png"
	for k,v in ipairs({table.unpack(items,2)}) do
		string = string .. ";pics/arrow-right.png;" ..  "tribes/" .. tribename .. "/" .. v  .. "/menu.png"
	end
	return rt(string, p(text))
end


--  =======================================================
--  *************** Dependencies functions ****************
--  =======================================================

-- RST
-- .. function:: building_help_depencencies_ware(tribename, items, ware)
--
--    Formats a chain of ware dependencies for the help window
--
--    :arg tribename: e.g. "barbarians".
--    :arg items: an array with ware and building names,
--                            e.g. {"constructionsite", "trunk"}
--    :arg warename: the internal name of the ware to use as a title.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_dependencies_ware(tribename, items, warename)
	local ware_descr = wl.Game():get_ware_description(tribename, warename)
	return dependencies(tribename, items, ware_descr.descname)
end


-- RST
-- .. function:: building_help_depencencies_building(tribename, items, building)
--
--    Formats a chain of ware dependencies for the help window
--
--    :arg tribename: e.g. "barbarians".
--    :arg items: an array with ware and building names,
--                            e.g. {"constructionsite", "trunk"}
--    :arg buildingname: the internal name of the building to use as a title.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_dependencies_building(tribename, items, buildingname)
	local building_descr = wl.Game():get_building_description(tribename,buildingname)
	return dependencies(tribename, items, building_descr.descname)
end



-- RST
-- .. function:: building_help_depencencies_building(tribename, items, building)
--
--    Formats a chain of ware dependencies for the help window
--
--    :arg tribename: e.g. "barbarians".
--    :arg items: an array with ware and building names,
--                            e.g. {"constructionsite", "trunk"}
--    :arg warename: the internal name of the ware to use in the title.
--    :arg building: the internal name of the building to use in the title.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_dependencies_ware_building(tribename, items, warename, buildingname)
	local building_descr = wl.Game():get_building_description(tribename,buildingname)
	local ware_descr = wl.Game():get_ware_description(tribename, warename)
	return dependencies(tribename, items, _"%1$s from: %2$s":bformat(ware_descr.descname, building_descr.descname))
end


-- RST
-- .. function:: building_help_depencencies_ware(tribename, items, ware)
--
--    Formats a chain of ware dependencies for the help window. First item is a mining resource.
--
--    :arg tribename: e.g. "barbarians".
--    :arg items: an array with ware and building names,
--                            e.g. {"constructionsite", "trunk"}
--    :arg warename: the internal name of the ware to use in the title.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_dependencies_resi(tribename, items, warename)
	local ware_descr = wl.Game():get_ware_description(tribename, warename)
	return dependencies_resi(tribename, items, ware_descr.descname)
end

-- RST
-- .. function:: dependencies_training(tribename, building_description, interim1, interim2)
--
--    Creates a dependencies line for soldiers in a training site.
--
--    :arg tribename: name of the tribe.
--    :arg building_description: the trainingsite's building description from C++
--    :arg interim1: the soldier level trained to in the first line, e.g. "untrained+evade".
--    :arg interim1: the soldier level trained from in the second line, e.g. "fulltrained-evade".
--    :returns: a row of pictures connected by arrows.
--
function dependencies_training(tribename, building_description, interim1, interim2)
	return
		rt(h2(_"Dependencies")) ..
		dependencies_basic({
			"tribes/" .. tribename .. "/soldier/untrained.png",
			"tribes/" .. tribename .. "/" .. building_description.name  .. "/menu.png",
			"tribes/" .. tribename .. "/soldier/" .. interim1 .. ".png"}) ..
		dependencies_basic({
			"tribes/" .. tribename .. "/soldier/" .. interim2 .. ".png",
			"tribes/" .. tribename .. "/" .. building_description.name  .. "/menu.png",
			"tribes/" .. tribename .. "/soldier/fulltrained.png"})
end


-- RST
-- .. function:: dependencies_training_food
--
--    Creates dependencies lines for food in training sites.
--
--    :arg tribename: name of the tribe.
--    :arg foods: an array of arrays with food items. Outer array has "and" logic and
--	          will appear from back to front, inner arrays have "or" logic
--    :returns: a list of food descriptions with images
--
function dependencies_training_food(tribename, foods)
	local result = ""
	for countlist, foodlist in pairs(foods) do
		local images = ""
		local text = ""
		for countfood, food in pairs(foodlist) do
			local ware_descr = wl.Game():get_ware_description(tribename, food)
			if(countfood > 1) then
				images = images .. ";"
				text = _"%1$s or %2$s":bformat(text, ware_descr.descname)
			else
				text = ware_descr.descname
			end
			images = images .. "tribes/" .. tribename .. "/" .. ware_descr.name .. "/menu.png"
		end
		if(countlist > 1) then
			text = _"%s and":bformat(text)
		end
		result = image_line(images, 1, p(text)) .. result
	end
	return result
end


-- RST
-- .. function:: dependencies_training_weapons(tribename, building_description, weapons, manufacturer)
--
--    Creates a dependencies line for any number of weapons.
--
--    :arg tribename: name of the tribe.
--    :arg building_description: the trainingsite's building description from C++
--    :arg weapons: an array of weapon names
--    :arg manufacturer: the name of the building manufacturing the weapons
--    :returns: a list weapons images with the producing and receiving building
--
function dependencies_training_weapons(tribename, building_description, weapons, manufacturer)
	local manufacturer_descr = wl.Game():get_building_description(tribename, manufacturer)
	local weaponsstring = ""
	for count, weapon in pairs(weapons) do
		if(count > 1) then
			weaponsstring = weaponsstring .. ";"
		end
		weaponsstring = weaponsstring .. "tribes/" .. tribename .. "/" .. weapon .. "/menu.png"
	end
	return dependencies_basic({
			"tribes/" .. tribename .. "/" .. manufacturer_descr.name  .. "/menu.png",
			weaponsstring,
			"tribes/" .. tribename .. "/" .. building_description.name  .. "/menu.png",
		}) .. rt(p(_"Provided by: %s":bformat(manufacturer_descr.descname)))
end


--  =======================================================
--  ************* Main buildinghelp functions *************
--  =======================================================

-- RST
-- .. function building_help_general_string(tribename, building_description, resourcename, purpose, note[, working_radius])
--
--    Creates the string for the general section in building help
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_description: The building's building description from C++
--    :arg resourcename: The name of a representative resource that this building produces
--    :arg purpose: A string explaining the purpose of the building
--    :arg purpose: A string with a note about the building. Use an empty string if you don't want to add a note.
--    :arg working_radius: The int working radius of the building. This paramater is optional.
--    :returns: rt of the formatted text
--
function building_help_general_string(tribename, building_description, resourcename, purpose, note, working_radius)
	-- Need to get the building description again to make sure we have the correct type, e.g. "productionsite"
	local building_description = wl.Game():get_building_description(tribename, building_description.name)
	local result = rt(h2(_"General"))
	result = result .. rt(h3(_"Purpose:")) ..
		image_line("tribes/" .. tribename .. "/" .. resourcename  .. "/menu.png", 1, p(purpose))
	if (note ~= "") then	result = result .. rt(h3(_"Note:")) .. rt(p(note)) end

	--result = result .. text_line(_"TEST TODO remove this when done:", building_description.type)

	if(building_description.type == "productionsite") then
		if (working_radius) then result = result .. text_line(_"Working radius:", working_radius) end
	elseif(building_description.type == "warehouse") then
		result = result .. rt(h3(_"Healing:")
	-- TODO heal_per_second = nil, why?
			.. p(_"Garrisoned soldiers heal %s per second":bformat(building_description.heal_per_second)))
		result = result .. text_line(_"Conquer range:", building_description.conquers)
	elseif(building_description.type == "militarysite") then
		result = result .. rt(h3(_"Healing:")
			.. p(_"Garrisoned soldiers heal %s per second":bformat(building_description.heal_per_second)))
		result = result .. text_line(_"Capacity:", building_description.max_number_of_soldiers)
		result = result .. text_line(_"Conquer range:", building_description.conquers)
	elseif(building_description.type == "trainingsite") then
		result = result .. rt(h2(_"Training"))
		result = result .. text_line(_"Capacity:", building_description.max_number_of_soldiers)
		if(building_description.max_attack > 0) then
			result = result .. text_line(_"Attack:", _"Trains ‘%1$s’ from %2$s up to %3$s":
				bformat(_"Attack", building_description.min_attack, building_description.max_attack))
		end
		if(building_description.max_defense > 0) then
			result = result .. text_line(_"Defense:", _"Trains ‘%1$s’ from %2$s up to %3$s":
				bformat(_"Defense", building_description.min_defense, building_description.max_defense))
		end
		if(building_description.max_evade > 0) then
			result = result .. text_line(_"Evade:", _"Trains ‘%1$s’ from %2$s up to %3$s":
				bformat(_"Evade", building_description.min_evade, building_description.max_evade))
		end
		if(building_description.max_hp > 0) then
			-- TRANSLATORS: %1$s = Health, Evade, Attack or Defense. %2$s and %3$s are numbers.
			result = result .. text_line(_"Health:", _"Trains ‘%1$s’ from %2$s up to %3$s":
				bformat(_"Health", building_description.min_hp, building_description.max_hp))
		end
	end
	result = result .. text_line(_"Vision range:", building_description.vision_range)
	return result
end


-- RST
-- .. function building_help_lore_string(tribename, building_description, flavourtext[, author])
--
--    Displays the building's main image with a flavour text.
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_description: The building description we get from C++
--    :arg flavourtext: e.g. "Catches fish in the sea".
--    :arg author: e.g. "Krumta, carpenter of Chat'Karuth". This paramater is optional.
--    :returns: rt of the image with the text
--
function building_help_lore_string(tribename, building_description, flavourtext, author)
	local result = rt(h2(_"Lore")) ..
		rt("image=tribes/" .. tribename .. "/" .. building_description.name  .. "/" .. building_description.name .. "_i_00.png", p(flavourtext))
		if author then
			result = result .. rt("text-align=right",p("font-size=10 font-style=italic", author))
		end
	return result
end


-- RST
-- .. function:: building_help_inputs(tribename, building_description, buildinglist, ware)
--
--    The input buildings of a ware
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_description: The building description we get from C++
--    :arg buildinglist: A table of building names that produce the ware
--    :arg warename: The name of the ware for which the buildings are displayed
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_inputs(tribename, building_description, buildinglist, warename)
	local building_description = wl.Game():get_building_description(tribename, building_description.name)
	local result = ""
	result = result .. rt(h3(_"Incoming:"))

	-- TODO get the buildinglist with the help of the resource
	for j, building in ipairs(buildinglist) do
		result = result .. building_help_dependencies_ware_building(
			tribename, {building, ware, building_description.name}, warename, building
		)
	end

	result = result .. rt(h2("Inputs test"))

	for i, ware in ipairs(building_description.inputs) do
		result = result .. rt(p(ware))
	end

	result = result .. rt(h2("End Inputs test"))
	return result
end


-- RST
-- .. function:: building_help_outputs(tribename, building_description)
--
--    The output wares of a building
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_description: The building description we get from C++
--    :arg buildinglist: A table of building names that produce the ware
--    :arg is_basic: True if this is a basic productionsite like a quarry that has no input wares.
--    :returns: an rt string with images describing a chain of ware/building dependencies
--
function building_help_outputs(tribename, building_description, buildinglist, is_basic)
	local building_description = wl.Game():get_building_description(tribename, building_description.name)
	local result = ""

	-- TODO get is_basic from [aihints] in conf, so we can define "Collects" from the output?
	-- or check if the list of input wares is empty
	if(is_basic) then
		result = result .. rt(h3(_"Collects:"))

		for i, ware in ipairs(building_description.output_ware_types) do
			result = result ..
				building_help_dependencies_ware(tribename, {building_description.name, ware}, ware)
		end
	elseif(building_description.ismine) then
		-- TRANSLATORS: This is a verb (The miner mines)
		result = result .. rt(h3(_"Mines:"))

		for i, ware in ipairs(building_description.output_ware_types) do
			result = result ..
				building_help_dependencies_resi(tribename, {"resi_"..ware.."2", building_description.name, ware}, ware)
		end
	else
		result = result .. rt(h3(_"Produces:"))
		for i, ware in ipairs(building_description.output_ware_types) do
			result = result ..
				building_help_dependencies_ware(tribename, {building_description.name, ware}, ware)
		end
	end

	result = result .. rt(h3(_"Outgoing:"))
	for i, ware in ipairs(building_description.output_ware_types) do
		-- TODO get the buildinglist with the help of the resource
		for j, building in ipairs(buildinglist) do
			result = result ..
				building_help_dependencies_building(tribename, {ware, building}, building)
		end
	end
	return result
end


-- Helper function for building_help_building_section
function building_help_building_line(tribename, ware, amount)
	local ware_descr = wl.Game():get_ware_description(tribename, ware)
	amount = tonumber(amount)
	local image = "tribes/" .. tribename .. "/" .. ware  .. "/menu.png"
	local result = ""
	local imgperline = 6
	local temp_amount = amount

	while (temp_amount > imgperline) do
		result = result .. image_line(image, imgperline)
		temp_amount = temp_amount - imgperline
	end
	result = image_line(image, temp_amount, p(_"%1$dx %2$s":bformat(amount, ware_descr.descname))) .. result
	return result

end

-- RST
--
-- .. function:: building_help_building_section(tribename, building_description[, upgraded_from, former_buildings])
--
--    Formats the "Building" section in the building help: Upgrading info, costs and space required
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_description: The building description we get from C++
--    :arg upgraded_from: The building name that this building is usually upgraded from.
--                        Leave blank if this is a basic building.
--    :former_buildings:  A table of building names representing the chain of buildings that this
--                        building was upgraded from. This is used to calculate cumulative building 
--                        and dismantle costs.
--    :returns: an rt string describing the building section
--
function building_help_building_section(tribename, building_description, upgraded_from, former_buildings)

	local result = rt(h2(_"Building"))

	-- Space required
	if (building_description.ismine) then
		result = result .. text_line(_"Space required:",_"Mine plot","pics/mine.png")
	elseif (building_description.isport) then
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

	-- Upgraded from
	if (building_description.buildable or building_description.enhanced) then
		if (building_description.enhanced) then
			local former_building = nil
			if (upgraded_from) then
				former_building = wl.Game():get_building_description(tribename, upgraded_from)
				result = result .. text_line(_"Upgraded from:", former_building.descname)
			else
				result = result .. text_line(_"Upgraded from:", _"Unknown")
			end

			for ware, amount in pairs(building_description.enhancement_cost) do
				result = result .. building_help_building_line(tribename, ware, amount)
			end

			-- Cost Cumulative
			result = result .. rt(h3(_"Cost Cumulative:"))
			local warescost = {}
			for ware, amount in pairs(building_description.enhancement_cost) do
				if (warescost[ware]) then
					warescost[ware] = warescost[ware] + amount
				else
					warescost[ware] = amount
				end
			end

			for index, former in pairs(former_buildings) do
				former_building = wl.Game():get_building_description(tribename, former)
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
					result = result .. building_help_building_line(tribename, ware, amount)
				end
			else
				result = result .. rt(p(_"Unknown"))
			end

			-- Dismantle yields
			result = result .. rt(h3(_"Dismantle yields:"))
			local warescost = {}
			for ware, amount in pairs(building_description.returned_wares_enhanced) do
				if (warescost[ware]) then
					warescost[ware] = warescost[ware] + amount
				else
					warescost[ware] = amount
				end
			end
			for index, former in pairs(former_buildings) do
				former_building = wl.Game():get_building_description(tribename, former)
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
					result = result .. building_help_building_line(tribename, ware, amount)
				end
			else
				result = result .. rt(p(_"Unknown"))
			end

		-- Buildable
		else

			-- Build Cost
			result = result .. rt(h3(_"Build Cost:"))
			for ware, amount in pairs(building_description.build_cost) do
				result = result .. building_help_building_line(tribename, ware, amount)
			end

			-- Dismantle yields
			result = result .. rt(h3(_"Dismantle yields:"))
			for ware, amount in pairs(building_description.returned_wares) do
				result = result .. building_help_building_line(tribename, ware, amount)
			end
		end

		-- Upgradeable to
		if (building_description.enhancements[1]) then
			for i, building in ipairs(building_description.enhancements) do
				result = result .. text_line(_"Upgradeable to:", building_description.enhancements[i].descname)
				for ware, amount in pairs(building_description.enhancements[i].enhancement_cost) do
					result = result .. building_help_building_line(tribename, ware, amount)
				end
			end
		end
	end
	return result
end



-- RST
-- .. function building_help_crew_string(tribename, building_description, workernames[, toolname])
--
--    Displays a worker with an image
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_description: the building_description from C++.
--    :arg workernames: an array with the most basic worker in the last position, e.g. {"chief-miner", "miner"}.
--    :arg toolname: the name of the tool the workers use, e.g. "pick"
--    :returns: Workers/Crew section of the help file
--
function building_help_crew_string(tribename, building_description, workernames, toolname)
	-- Need to get the building description again to make sure we have the correct type, e.g. "productionsite"
	local building_description = wl.Game():get_building_description(tribename, building_description.name)
	local result = ""

	if(building_description.type == "productionsite" or building_description.type == "trainingsite") then

		result = result .. rt(h2(_"Workers")) .. rt(h3(_"Crew required:"))

		-- TODO this is empty
		--for i, building in ipairs(building_description.working_positions) do
		--	result = result .. text_line(_"Found worker!", building_description.working_positions[i].descname)
		--end

		local worker_descr = nil
		local becomes_descr = nil
		local number_of_workers = 0

		for i, worker in ipairs(workernames) do
			worker_descr = wl.Game():get_worker_description(tribename, worker)
			becomes_descr = worker_descr.becomes
			number_of_workers = number_of_workers + 1

			if(becomes_descr) then
				result = result .. image_line("tribes/" .. tribename .. "/" .. worker  .. "/menu.png", 1,
					p(_"%s or better":bformat(worker_descr.descname)))
			else
				result = result .. image_line("tribes/" .. tribename .. "/" .. worker  .. "/menu.png", 1,
					p(worker_descr.descname))
			end
		end

		if(toolname) then result = result .. building_help_tool_string(tribename, toolname, number_of_workers) end

		if(becomes_descr) then

			result = result .. rt(h3(_"Experience levels:"))
			local exp_string = _"%s to %s (%s EP)":format(
					worker_descr.descname,
					becomes_descr.descname,
					worker_descr.level_experience
				)

			worker_descr = becomes_descr
			becomes_descr = worker_descr.becomes
			if(becomes_descr) then
				exp_string = exp_string .. "<br>" .. _"%s to %s (%s EP)":format(
						worker_descr.descname,
						becomes_descr.descname,
						worker_descr.level_experience
					)
			end
			result = result ..  rt("text-align=right", p(exp_string))
		end
	end

	return result
end


-- RST
-- .. function building_help_tool_string(tribename, toolname)
--
--    Displays a tool with an intro text and image
--
--    :arg tribename: e.g. "barbarians".
--    :arg toolname: e.g. "felling_axe".
--    :arg no_of_workers: the number of workers using the tool; for plural formatting.
--    :returns: text_line for the tool
--
function building_help_tool_string(tribename, toolname, no_of_workers)
	local ware_descr = wl.Game():get_ware_description(tribename,toolname)
	return text_line((ngettext("Worker uses:","Workers use:", no_of_workers)),
		ware_descr.descname, "tribes/" .. tribename .. "/" .. toolname  .. "/menu.png")
end
