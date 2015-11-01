include "tribes/scripting/help/format_help.lua"

-- RST
-- worker_help.lua
-- ---------------

-- Functions used in the ingame worker help windows for formatting the text and pictures.


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
		result = result .. help_tool_string(tribe, toolnames, 1)
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


return {
   func = function(tribename, worker_description)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      return worker_help_string(tribe, worker_description)
   end
}
