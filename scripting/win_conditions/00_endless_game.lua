-- =======================================================================
--                         An endless game without rules
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

local wc_name = _ "Endless Game"
local wc_desc = _"This is an endless game without rules."
return {
	name = wc_name,
	description = wc_desc,
	-- This function just cares about players that were defeated and gives them
	-- full vision
	func = function()
		-- Find all valid players
		local plrs = {}
		valid_players(plrs)

		-- send a message with the game type to all players
		for idx, p in ipairs(plrs) do
			p:send_message(wc_name, wc_desc)
		end

		-- Iterate all players, if one is defeated, remove him
		-- from the list, send him a defeated message and give him full vision
		repeat
			sleep(5000)
			check_player_defeated(plrs, _ "You are defeated!",
				_ "You have nothing to command left. If you want, you may " ..
				  "continue as spectator.")
		until count_factions(plrs) < 1
	
	end
}
