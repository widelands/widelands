-- NOCOM we should have a common include for the helper functions.

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
-- .. function:: dependencies(items[, text = nil])
--
--    Creates a dependencies line of any length.
--
--    :arg items: ware, worker and/or building descriptions in the correct order
--                from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies(items, text)
	if not text then
		text = ""
	end
	local string = "image=" .. items[1].icon_name
	for k,v in ipairs({table.unpack(items,2)}) do
		string = string .. ";pics/arrow-right.png;" ..  v.icon_name
	end
	return rt(string, p(text))
end

--  =======================================================
--  ************* Main worker help functions *************
--  =======================================================

-- RST
-- .. function ware_help_string(tribe, ware_description)
--
--    Displays the ware with a helptext and an image
--
--    :arg tribe: The :class:`LuaTribeDescription` for a tribe that uses this ware.
--    :arg ware_description: the worker_description from C++.
--    :returns: Help string for the ware
--
function ware_help_string(tribe, ware_description)
	include(ware_description.directory .. "helptexts.lua")

	local purpose_text = ware_helptext()
	if (purpose_text ~= "") then
		purpose_text = purpose_text .. " "
	end
	purpose_text = ware_helptext() .. ware_helptext(tribe.name)

	local result = rt(h2(_"Purpose")) ..
		rt("image=" .. ware_description.icon_name, p(purpose_text))

	result = result .. rt(h3(_"Producers:"))

	for j, building in ipairs(ware_description.producers) do
		if (tribe:has_building(building.name)) then
			result = result .. dependencies(
				{building, ware_description},
				building.descname
			)
		end
	end

	result = result .. rt(h3(_"Consumers:"))

	for j, building in ipairs(ware_description.consumers) do
		if (tribe:has_building(building.name)) then
			result = result .. dependencies(
				{building, ware_description},
				building.descname
			)
		end
	end

	-- TODO(GunChleoc): Split into purpose and note
	-- We also want the ware quantity info collected while loading the tribes.
	return result
end


return {
   func = function(tribename, ware_description)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
	   return ware_help_string(tribe, ware_description)
   end
}
