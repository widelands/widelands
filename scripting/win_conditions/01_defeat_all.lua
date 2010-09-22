-- =======================================================================
--                         Defeat all Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

local wc_name = _ "Autocrat"
local wc_desc = _ "The tribe or team that can defeat all others wins the game!"
return {
	name = wc_name,
	description = wc_desc,
	func = function()
		local plrs = wl.Game().players

		broadcast(plrs, wc_name, wc_desc)

		-- Iterate all players, if one is defeated, remove him
		-- from the list, send him a defeated message and give him full vision
		repeat
			sleep(5000)
			check_player_defeated(plrs, _ "You are defeated!",
				_ ("You have nothing to command left. If you want, you may " ..
				   "continue as spectator."))
		until count_factions(plrs) <= 1

		-- Send congratulations to all remaining players
		broadcast(plrs, 
				_ "Congratulations!",
				_ "You have won this game!",
				{popup = true}
		)

	end,
}
