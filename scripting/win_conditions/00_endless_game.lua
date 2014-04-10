-- =======================================================================
--                         An endless game without rules
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_condition_texts.lua"

local wc_name = _ "Endless Game"
local wc_version = 1
local wc_desc = _"This is an endless game without rules."
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
			check_player_defeated(plrs, lost_game.title, lost_game.body, wc_name, wc_version)
		until count_factions(plrs) < 1

	end
}
