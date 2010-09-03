-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

local wc_name = _ "Collectors"
local wc_desc = _ (
	"You get points for all wares stored in your headquarters or warehouses. " ..
	"The player with the highest number of wares at the end of 4 hours wins " ..
	"the game."
)
return {
	name = wc_name,
	description = wc_desc,
	func = function()
		-- Find all valid players
		local plrs = {}
		valid_players(plrs)

		-- send a message with the game type to all players
		for idx, p in ipairs(plrs) do
			p:send_message(wc_name, wc_desc)
		end

		local remaining_time = 60 * 4
		function _calc_points(p)
			local bs = array_combine(
				p:get_buildings("headquarters"), p:get_buildings("warehouse")
			)
			local rv = 0
			for idx,b in ipairs(bs) do
				for wname, count in pairs(b:get_wares("all")) do
					rv = rv + count
				end
			end
			return rv
		end

		function _send_state()
			local msg = _("The game will end in %i minutes."):format(remaining_time)
			msg = msg .. "\n\n"

			for idx, p in ipairs(plrs) do
				msg = msg .. _("The player %i has currently %i points\n"):
					format(p.number, _calc_points(p))
			end

			for idx, p in ipairs(plrs) do
				p:send_message(_ "Status", msg)
			end
		end

		sleep(1000)

		while true do
			local runs = 0
			repeat
				sleep(5000)
				check_player_defeated(plrs, _ "You are defeated!",
					_ "You have nothing to command left. If you want, you may " ..
					  "continue as spectator.")
				runs = runs + 1
			until runs >= 120 -- 120 * 5000ms = 600000 ms = 10 min

			remaining_time = remaining_time - 10

			_send_state()

			-- Game ended?
			if remaining_time <= 0 then
				local points = {}
				for idx,plr in ipairs(plrs) do
					points[#points + 1] = { plr, _calc_points(plr) }
				end
				table.sort(points, function(a,b) return a[2] < b[2] end)
				for i=1,#points-1 do
					points[i][1]:send_message(_"You lost", _"You have lost this game!",{popup = true})
				end
				points[#points][1]:send_message(_"You won!", _"You are the winner of this game!",{popup = true})
				break
			end
		end
	end
}
