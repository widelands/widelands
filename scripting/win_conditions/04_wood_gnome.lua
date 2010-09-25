-- =======================================================================
--                       Wood Gnome win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

local wc_name = _ "Wood Gnome"
local wc_desc = _
[[As wood gnome you like big forests, so your task is, to have more trees on
your territory than any other player. The game will end after 4 hours of
playing. The one with the most trees at that point will win the game.]]

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
				if not f.has_movecaps_swim(f) then
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
			local owner = 0
			if f.owners[1] then
				owner = f.owners[1].number
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
		local playerpoints = _calc_points()
		local msg = _("The game will end in %i minutes."):format(remaining_time)
		msg = msg .. "\n\n"
		msg = msg .. _ ("Player overview:")
		for idx,plr in ipairs(plrs) do
			msg = msg .. "\n"
			msg = msg .. _ ("%s has "):format(plr.name)
			msg = msg .. _ ("%i trees at the moment ."):format(playerpoints[plr.number])
		end

		broadcast(plrs, _ "Status", msg)
	end

	-- Start a new coroutine that checks for defeated players
	run(function()
		sleep(5000)
		check_player_defeated(plrs, _ "You are defeated!",
			_ ("You have nothing to command left. If you want, you may " ..
				"continue as spectator."))
	end)

	-- Install statistics hook
	if hooks == nil then hooks = {} end
	hooks.custom_statistic = {
		name = _ "Trees owned",
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
	msg = msg .. _ ("Player overview:")
	for idx,plr in ipairs(plrs) do
		msg = msg .. "\n"
		msg = msg .. _ ("%s had "):format(plr.name)
		msg = msg .. _ ("%i trees."):format(playerpoints[plr.number])
	end
	msg = msg .. "\n\n"
	msg = msg .. _ ("The winner is %s "):format(points[#points][1].name)
	msg = msg .. _ ("with %i trees."):format(playerpoints[points[#points][1].number])
	local privmsg = ""
	for i=1,#points-1 do
		privmsg = _ ("You have lost this game!")
		privmsg = privmsg .. msg
		points[i][1]:send_message(_"You lost!", privmsg, {popup = true})
	end
	privmsg = _ ("You are the winner of this game!")
	privmsg = privmsg .. msg
	points[#points][1]:send_message(_"You won!", privmsg, {popup = true})
end
}
