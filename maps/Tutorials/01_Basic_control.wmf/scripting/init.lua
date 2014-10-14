-- =======================================================================
--                       Tutorial 01: Basic Control
-- =======================================================================

plr = wl.Game().players[1]
map = wl.Game().map

set_textdomain("Basic_control.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

-- Constants
first_lumberjack_field = map:get_field(16,10)
first_quarry_field = map:get_field(8,12)
second_quarry_field = map:get_field(5,10)

-- Global variables
registered_player_immovables = {}
terminate_bad_boy_sentinel = false
immovable_is_legal = function(i) return false end

include "map:scripting/starting_conditions.lua"

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"
include "map:scripting/helper_functions_demonstration.lua"

include "map:scripting/mission_thread.lua"
