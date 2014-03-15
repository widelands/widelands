-- RST
-- format_help.lua
-- ---------------
--
-- Functions used in the ingame help windows for formatting the text and pictures.

-- RST
-- .. function:: dependencies(images[, text = nil])
--
--    Creates a dependencies line of any length.
--
--    :arg images: images in the correct order from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies(images, text)
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

-- RST
-- .. function building_help_general_string(tribename, resourcename, amount, purpose, working_radius, conquer_range, vision_range)
--
--    Creates the string for the general section in building help
--
--    :arg tribename: e.g. "barbarians".
--    :arg resourcename: The resource this building produces
--    :arg working_radius: The owrking radious of the building
--    :arg conquer_range: The conquer range of the building
--    :arg vision_range: The vision range of the building
--    :returns: rt of the formatted text
--
function building_help_general_string(tribename, resourcename, purpose, working_radius, conquer_range, vision_range)
	return rt(h2(_"General")) .. rt(h3(_"Purpose:")) ..
	image_line("tribes/" .. tribename .. "/" .. resourcename  .. "/menu.png", 1, p(purpose)) ..
	text_line(_"Working radius:", working_radius) ..
	text_line(_"Conquer range:", conquer_range) ..
	text_line(_"Vision range:", vision_range)
end


-- RST
-- .. function building_help_lore_string(tribename, buildingname, flavourtext[, author])
--
--    Displays the building's main image with a flavour text.
--
--    :arg tribename: e.g. "barbarians".
--    :arg buildingname: e.g. "lumberjacks_hut".
--    :arg flavourtext: e.g. "Catches fish in the sea".
--    :arg author: e.g. "Krumta, carpenter of Chat'Karuth". This paramater is optional.
--    :returns: rt of the image with the text
--
function building_help_lore_string(tribename, buildingname, flavourtext, author)
	local result = rt(h2(_"Lore")) ..
		rt("image=tribes/" .. tribename .. "/" .. buildingname  .. "/" .. buildingname .. "_i_00.png", p(flavourtext))
	if author then
		result = result .. rt("text-align=right",p("font-size=10 font-style=italic", author))
	end
	return result
end

-- .. help_building_line(tribe, material, material_str, number)
--
--    Creates an image_line with a number formatted string.
--
--    :arg tribe: name of tribe in the file system.
--    :arg material: name of material in the file system.
--    :arg material_str: containing number placeholder + name of material fetched with ngettext previously.
--    :arg number: the number used for ngettext in material_str
--    :returns: image_line.
--
function help_building_line(tribe, material, material_str, number)
	local image = "tribes/" .. tribe .. "/" .. material .. "/menu.png"
	if number <=6 then
		return image_line(image,number,p(material_str:format(number)))
   end
   if number <=12 then
      return image_line(image,6,p(material_str:format(number))) .. image_line(image, number-6)
   else
      return image_line(image,6,p(material_str:format(number))) .. image_line(image, 6) .. image_line(image, number-12)
   end
end


-- RST
-- .. function:: building_help_collecting_list(tribename, building_resource)
--
--    Formats the list of resources a building collects for the help window
--
--    :arg tribename: tribename, e.g. "barbarians".
--    :arg building_resource: an array of arrays with building and resource names,
--                            e.g. {{"constructionsite", "trunk"},{"burners_house", "trunk"}}
--    :returns: an rt string describing a list of collected resources, or an empty string if building_resource == {}
--
function building_help_collecting_list(tribename, building_resource)

	if (table.getn(building_resource) == 0) then return "" end

	local result =rt(h3(_"Collects:"));
	for i,line in ipairs({unpack(building_resource,1)}) do
		result = result ..

			dependencies({"tribes/" .. tribename .. "/" .. line[1]  .. "/menu.png",
			"tribes/" .. tribename .. "/" .. line[2]  .. "/menu.png"},
			p(line[2])) -- todo get localised name from tribe's main conf!
	end
	return result
end


-- RST
-- .. function:: building_help_incoming_list(tribename, building_resource)
--
--    Formats the list of incoming resources for a building for the help window
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_resource: an array of arrays with building and resource names,
--                            e.g. {{"constructionsite", "trunk"},{"burners_house", "trunk"}}
--    :returns: an rt string describing a list of incoming resources, or an empty string if building_resource == {}
--
function building_help_incoming_list(tribename, building_resource)

	if (table.getn(building_resource) == 0) then return "" end

	local result =rt(h3(_"Consumes:"));
	for i,line in ipairs({unpack(building_resource,1)}) do
		result = result ..

			dependencies({"tribes/" .. tribename .. "/" .. line[1]  .. "/menu.png",
			"tribes/" .. tribename .. "/" .. line[2]  .. "/menu.png"},
			p(line[2])) -- todo get localised name from tribe's main conf!
	end
	return result
end


-- RST
-- .. function:: building_help_outgoing_list(tribename, building_resource)
--
--    Formats the list of outgoing resources for a building for the help window
--
--    :arg tribename: e.g. "barbarians".
--    :arg building_resource: an array of arrays with building and resource names,
--                            e.g. {{"constructionsite", "trunk"},{"burners_house", "trunk"}}
--    :returns: an rt string describing a list of outgoing resources, or an empty string if building_resource == {}
--
function building_help_outgoing_list(tribename, building_resource)

	if (table.getn(building_resource) == 0) then return "" end

	local result =rt(h3(_"Produces:"));
	for i,line in ipairs({unpack(building_resource,1)}) do
		result = result ..

			dependencies({"tribes/" .. tribename .. "/" .. line[2]  .. "/menu.png",
			"tribes/" .. tribename .. "/" .. line[1]  .. "/menu.png"},
			p(line[1])) -- todo get localised name from tribe's main conf!
	end
	return result
end


-- RST
-- .. function building_help_size_string(tribename, buildingname)
--
--    Creates a text_line that describes the building's size in a help text.
--
--    :arg tribename: e.g. "barbarians".
--    :arg buildingname: e.g. "lumberjacks_hut".
--    :returns: "Space required" header followed by size description text and image.
--
function building_help_size_string(tribename, buildingname)

  local building_descr = wl.Game():get_building_description(tribename,buildingname)

  if(building_descr.ismine) then
	return text_line(_"Space required:",_"Mine plot","pics/mine.png")
  elseif(building_descr.isport) then
	return text_line(_"Space required:",_"Port plot","pics/port.png")
  else
	if (building_descr.size == 1) then
 		return text_line(_"Space required:",_"Small plot","pics/small.png")
	elseif (building_descr.size == 2) then
  		return text_line(_"Space required:",_"Medium plot","pics/medium.png")
	elseif (building_descr.size == 3) then
		return text_line(_"Space required:",_"Big plot","pics/big.png")
	else
		return p(_"Space required:" .. _"Unknown")
	end
  end
end


-- RST
--
-- TODO: Causes panic in image_line if resource does not exist
--
-- .. function:: building_help_cost_list(tribename, resource_amount)
--
--    Formats a list of resource costs
--
--    :arg tribename: e.g. "barbarians".
--    :arg resource_amount: an array of arrays with resource names and the amoutn of each resource,
--                            e.g. {{"trunk", 2},{"raw_stone", 1}}
--    :returns: an rt string describing a list of resource costs
--
function building_help_cost_list(tribename, resource_amount)

	local result =""
	for i,line in ipairs({unpack(resource_amount,1)}) do
		result = result ..
			image_line("tribes/" .. tribename .. "/" .. line[1]  .. "/menu.png",
				line[2], p(line[2] .. " ".. line[1]))
			-- todo get localised name from tribe's main conf, and add ngettext!
	end
	return result
end



-- RST
-- .. function building_help_crew_string(tribename, workername, amount)
--
--    Displays a worker with an image
--
--    :arg tribename: e.g. "barbarians".
--    :arg workername: e.g. "lumberjack".
--    :returns: image_line for the worker
--
function building_help_crew_string(tribename, workername)
	return image_line("tribes/" .. tribename .. "/" .. workername  .. "/menu.png", 1, p(workername))
	-- todo get localised name from tribe's main conf, and add ngettext!
end

-- RST
-- .. function building_help_tool_string(tribename, toolname)
--
--    Displays a tool with an intro text and image
--
--    :arg tribename: e.g. "barbarians".
--    :arg toolname: e.g. "felling_axe".
--    :returns: text_line for the tool
--
function building_help_tool_string(tribename, toolname)
	return text_line(_"Worker uses:", toolname, "tribes/" .. tribename .. "/" .. toolname  .. "/menu.png")
	-- todo get localised name from tribe's main conf, and add ngettext!
end
