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
--  ************* Main worker help functions *************
--  =======================================================

-- RST
-- .. function worker_help_string(worker_description)
--
--    Displays the worker with a helptext, an image and the tool used
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe
--                that we are displaying this help for.
--
--    :arg worker_description: the worker_description from C++.
--    :returns: Help string for the worker
--
function worker_help_string(tribe, worker_description)
	include(worker_description.directory .. "helptexts.lua")

	local result = rt(h2(_"Purpose")) ..
		rt("image=" .. worker_description.icon_name, p(worker_helptext()))

	-- Get the tools for the workers.
	local toolnames = {}
	for j, buildcost in ipairs(worker_description.buildcost) do
		if (buildcost ~= nil and tribe:has_ware(buildcost)) then
			toolnames[#toolnames + 1] = buildcost
		end
	end

	if(#toolnames > 0) then
		result = result .. worker_help_tool_string(toolnames)
	end

	-- TODO(GunChleoc): Add "enhanced from" info in one_tribe branch
	local becomes_description = worker_description.becomes
	if (becomes_description) then

		result = result .. rt(h3(_"Experience levels:"))
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
	return result
end


-- RST
-- .. function worker_help_tool_string(toolname)
--
--    Displays tools with an intro text and images
--
--    :arg toolnames: e.g. {"shovel", "basket"}.
--    :returns: image_line for the tools
--
function worker_help_tool_string(toolnames)
	local result = rt(h3(ngettext("Worker uses:","Workers use:", 1)))
	local game  = wl.Game();
	for i, toolname in ipairs(toolnames) do
		local ware_description = game:get_ware_description(toolname)
		result = result .. image_line(ware_description.icon_name, 1, p(ware_description.descname))
	end
	return result
end


return {
   func = function(tribename, worker_description)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      return worker_help_string(tribe, worker_description)
   end
}
