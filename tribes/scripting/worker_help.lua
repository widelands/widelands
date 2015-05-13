set_textdomain("tribes")

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
--    :arg worker_description: the worker_description from C++.
--    :returns: Help string for the worker
--
function worker_help_string(worker_description)
	local result = rt(h2(_"Lore")) ..
		rt("image=" .. worker_description.icon_name, p(worker_description.helptext))

	-- Get the tools for the workers.
	local toolnames = {}
	if(worker_description.buildable) then
		for j, buildcost in ipairs(worker_description.buildcost) do
			if( not (buildcost == "carrier" or buildcost == "none" or buildcost == nil)) then
				toolnames[#toolnames + 1] = buildcost
			end
		end
	end

	if(#toolnames > 0) then
		result = result .. worker_help_tool_string(worker_description.tribename, toolnames)
	end

	-- TODO(GunChleoc): Add "enhanced from" info in one_tribe branch
	local becomes_description = worker_description.becomes
	if(becomes_description) then

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
-- .. function worker_help_tool_string(tribename, toolname)
--
--    Displays tools with an intro text and images
--
--    :arg tribename: e.g. "barbarians".
--    :arg toolnames: e.g. {"shovel", "basket"}.
--    :returns: image_line for the tools
--
function worker_help_tool_string(tribename, toolnames)
	local result = rt(h3(ngettext("Worker uses:","Workers use:", 1)))
	local game  = wl.Game();
	for i, toolname in ipairs(toolnames) do
		local ware_description = game:get_ware_description(tribename, toolname)
		result = result .. image_line(ware_description.icon_name, 1, p(ware_description.descname))
	end
	return result
end


return {
   func = function(worker_description)
	return worker_help_string(worker_description)
  end
}
