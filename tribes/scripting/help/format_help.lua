include "scripting/formatting.lua"

-- RST
-- format_help.lua
-- ---------------

-- Functions used in the ingame help windows for formatting the text and pictures.

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


--  =======================================================
--  ********** Helper functions for dependencies **********
--  =======================================================

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


-- RST
-- .. function help_ware_amount_line(ware_description, amount)
--
--    Displays an amount of wares with name and images
--
--    :arg ware_description: The :class:`LuaWareDescription` for the ware type to be displayed
--    :arg amount: The amount to show as a number
--    :returns: image_line for the ware type and amount
--
function help_ware_amount_line(ware_description, amount)
	amount = tonumber(amount)
	local image = ware_description.icon_name
	local result = ""
	local imgperline = 6
	local temp_amount = amount

	while (temp_amount > imgperline) do
		result = result .. image_line(image, imgperline)
		temp_amount = temp_amount - imgperline
	end
	-- TRANSLATORS: %1$d is a number, %2$s the name of a ware, e.g. 12x Stone
	result = image_line(image, temp_amount, p(_"%1$dx %2$s":bformat(amount, ware_description.descname))) .. result
	return result
end

-- RST
-- .. function help_tool_string(tribe, toolname, no_of_workers)
--
--    Displays tools with an intro text and images
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe that uses the tools
--    :arg toolnames: e.g. {"shovel", "basket"}.
--    :arg no_of_workers: the number of workers using the tools; for plural formatting.
--    :returns: image_line for the tools
--
function help_tool_string(tribe, toolnames, no_of_workers)
	local result = ""
	local game  = wl.Game();
	for i, toolname in ipairs(toolnames) do
		if (tribe:has_ware(toolname)) then
			local ware_description = game:get_ware_description(toolname)
			result = result .. image_line(ware_description.icon_name, 1, p(ware_description.descname))
		end
	end
	if (result ~= "") then
		result = rt(h3(ngettext("Worker uses:","Workers use:", no_of_workers))) .. result
	end
	return result
end
