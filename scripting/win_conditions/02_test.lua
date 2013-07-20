-- =======================================================================
--                         Defeat all Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

use("aux", "win_condition_texts")

local wc_name = _ "Test Win"
local wc_version = 2
local wc_desc = _ "The tribe or team that can defeat all others wins the game!"
return {
	name = wc_name,
	description = wc_desc,
	func = function()
		local plrs = wl.Game().players

		broadcast(plrs, wc_name, wc_desc)
		sleep(15000)
		wl.game.report_result(plrs[1], true, 0, make_extra_data(plrs[1], wc_name, wc_version))
		table.remove(plrs, 1)

		broadcast_lost(plrs,
				wc_name,
				"Bravo!",
				{popup = true},
				wc_name, wc_version
		)

	end,
}
