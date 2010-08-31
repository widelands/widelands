-- =======================================================================
--                       Wood Gnome win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

return {
	name = _ "Wood Gnome",
	description = _ (
	"As wood gnome you like big forests, so your task is, to have more trees " ..
	"on your territory than any other player. The trees will be counted after " ..
	"4 hours of playing."
	),
	func = function()
		-- Find all valid players
		local plrs = {}
		valid_players(plrs)

		local remaining_time = 20 -- 4 hours

		local points = {}
		local playerpoints = {}
		function _calc_points()
			-- init the playerpoints for each player
			for idx,plr in ipairs(plrs) do
				playerpoints[plr.number] = 0
			end
			local mapwidth  = wl.map.get_width()
			local mapheight = wl.map.get_height()
			for x=0,mapwidth-1 do
				for y=0,mapheight-1 do
					local f = wl.map.Field(x,y)
					if f then
						-- check if field is owned by a player
						local owner = 0
						if f.owners[1] then
							owner = f.owners[1].number
							-- check if field has an immovable
							local imm = f.immovable
							if imm then
								-- check if immovable is a tree
								if imm:has_attribute("tree") then
								playerpoints[owner] = playerpoints[owner] + 1
								end
							end
						end
					end
				end
			end
			for idx,plr in ipairs(plrs) do
				points[#points + 1] = { plr, playerpoints[plr.number] }
			end
		end

		function _send_state()
			local msg = _("The game will end in %i minutes."):format(remaining_time)
			msg = msg .. "\n\n"

			for idx, p in ipairs(plrs) do
				p:send_message(_ "Status", msg)
			end
		end

		-- main loop
		while true do
			local runs = 0
			repeat
				sleep(5000)
				check_player_defeated(plrs, _ "You are defeated!",
					_ "You have nothing to command left. If you want, you may " ..
					  "continue as spectator.")
				runs = runs + 1
			until runs >= 60 -- 60 * 5000ms = 300000 ms = 5 minutes

			remaining_time = remaining_time - 5

			-- at the beginning send remaining time message only each 30 minutes
			-- if only 30 minutes or less are left, send each 5 minutes
			if (remaining_time < 30 or remaining_time % 30 == 0) and remaining_time > 0 then
				_send_state()
			end

			-- Do this, as soon as the 4 hours are over
			if remaining_time <= 0 then
				_calc_points()
				local msg = "\n\n"
				msg = msg .. _ ("Player overview:")
				for idx,plr in ipairs(plrs) do
					msg = msg .. "\n"
					msg = msg .. _ ("%s had "):format(plr.name)
					msg = msg .. _ ("%i trees."):format(playerpoints[plr.number])
				end
				table.sort(points, function(a,b) return a[2] < b[2] end)
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
				break
			end
		end
	end
}