-- =======================================================================
--                       Wood Gnome win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/table.lua"
include "scripting/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_condition_texts.lua"

local wc_name = _ "Wood Gnome"
local wc_version = 2
local wc_desc = _(
[[As wood gnome you like big forests, so your task is to have more trees on ]] ..
[[your territory than any other player. The game will end after 4 hours of ]] ..
[[playing. The one with the most trees at that point will win the game.]])
local wc_trees_owned = _"Trees owned"
return {
	name = wc_name,
	description = wc_desc,
	func = function()
	local plrs = wl.Game().players

	-- send a message with the game type to all players
	broadcast(plrs, wc_name, wc_desc)

	local remaining_time = 4 * 60 -- 4 hours

	-- Get all valueable fields of the map
	local fields = {}
   local map = wl.Game().map
	for x=0,map.width-1 do
		for y=0,map.height-1 do
			local f = map:get_field(x,y)
			if f then
				-- add this field to the list as long as it has not movecaps swim
				if not f:has_caps("swimmable") then
					fields[#fields+1] = f
				end
			end
		end
	end

	-- The function to calculate the current points.
	local _last_time_calculated = -100000
	local _plrpoints = {}
	local function _calc_points()
		local game = wl.Game()

		if _last_time_calculated > game.time - 5000 then
			return _plrpoints
		end

		-- init the playerpoints for each player
		for idx,plr in ipairs(plrs) do
			_plrpoints[plr.number] = 0
		end
		for idf,f in ipairs(fields) do
			-- check if field is owned by a player
			local owner = f.owner
			if owner then
				owner = owner.number
				-- check if field has an immovable
				local imm = f.immovable
				if imm then
					-- check if immovable is a tree
					if imm:has_attribute("tree") then
						_plrpoints[owner] = _plrpoints[owner] + 1
					end
				end
			end
		end

		_last_time_calculated = game.time
		return _plrpoints
	end

	local function _send_state()
		set_textdomain("win_conditions")
		local playerpoints = _calc_points()
		local msg = (ngettext("The game will end in %i minute.", "The game will end in %i minutes.", remaining_time))
				:format(remaining_time)
		msg = msg .. "\n\n"
		msg = msg .. game_status.body
		for idx,plr in ipairs(plrs) do
			msg = msg .. "\n"
			local trees = (ngettext ("%i tree", "%i trees", playerpoints[plr.number]))
					:format(playerpoints[plr.number])
			msg = msg ..  (_"%1$s has %2$s at the moment."):bformat(plr.name,trees)
		end

		broadcast(plrs, game_status.title, msg)
	end

	-- Start a new coroutine that checks for defeated players
	run(function()
		sleep(5000)
		check_player_defeated(plrs, lost_game.title, lost_game.body, wc_name, wc_version)
	end)

	-- Install statistics hook
	if hooks == nil then hooks = {} end
	hooks.custom_statistic = {
		name = wc_trees_owned,
		pic = "pics/genstats_trees.png",
		calculator = function(p)
			local pts = _calc_points(p)
			return pts[p.number]
		end,
	}

	-- main loop
	while true do
		sleep(300000) -- 5 Minutes
		remaining_time = remaining_time - 5

		-- at the beginning send remaining time message only each 30 minutes
		-- if only 30 minutes or less are left, send each 5 minutes
		if (remaining_time < 30 or remaining_time % 30 == 0)
				and remaining_time > 0 then
			_send_state()
		end

		if remaining_time <= 0 then break end
	end

	-- Game has ended
	local playerpoints = _calc_points()
	local points = {}
	for idx,plr in ipairs(plrs) do
		points[#points + 1] = { plr, playerpoints[plr.number] }
	end
	table.sort(points, function(a,b) return a[2] < b[2] end)

	local msg = "\n\n"
	msg = msg .. game_status.body
	for idx,plr in ipairs(plrs) do
		msg = msg .. "\n"
		local trees = (ngettext ("%i tree", "%i trees", playerpoints[plr.number])):format(playerpoints[plr.number])
		msg = msg ..  (_"%1$s had %2$s."):bformat(plr.name,trees)
	end
	msg = msg .. "\n\n"
	local trees = (ngettext ("%i tree", "%i trees", playerpoints[points[#points][1].number]))
			:format(playerpoints[points[#points][1].number])
	msg = msg ..  (_"The winner is %1$s with %2$s."):bformat(points[#points][1].name, trees)

	local privmsg = ""
	for i=1,#points-1 do
		privmsg = lost_game_over.title
		privmsg = privmsg .. msg
		points[i][1]:send_message(lost_game_over.body, privmsg)
		wl.game.report_result(points[i][1], 0, make_extra_data(points[i][1], wc_name, wc_version, {score=points[i][2]}))
	end
	privmsg = won_game_over.title
	privmsg = privmsg .. msg
	points[#points][1]:send_message(won_game_over.body, privmsg)
   wl.game.report_result(points[#points][1], 1,
      make_extra_data(points[#points][1], wc_name, wc_version, {score=points[#points][2]}))
end
}
