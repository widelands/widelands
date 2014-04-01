-- =======================================================================
--                         An endless game without rules
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_condition_texts.lua"

local wc_name    = _ "Endless Game (no fog)"
local wc_desc    = _ "This is an endless game without rules. Fog of war is disabled."
local wc_version = 1
return {
	name = wc_name,
	description = wc_desc,
	func = function()
		local plrs = wl.Game().players

		broadcast(plrs, wc_name, wc_desc)

		-- reveal the whole map for every player
		local game = wl.Game()
		local map = game.map
		local fields = {}
		local i = 1
		for x=0, map.width-1 do
			for y=0, map.height-1 do
				fields[i] = map:get_field(x,y)
				i = i + 1
			end
		end
		for idx, plr in ipairs(game.players) do
			plr:reveal_fields(fields)
		end

		-- Iterate all players, if one is defeated, remove him
		-- from the list, send him a defeated message and give him full vision
		repeat
			sleep(5000)
			check_player_defeated(plrs, lost_game.title, lost_game.body, wc_name, wc_version)
		until count_factions(plrs) < 1

	end
}
