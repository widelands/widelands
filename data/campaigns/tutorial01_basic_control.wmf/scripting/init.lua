-- =======================================================================
--                       Tutorial 01: Basic Control
-- =======================================================================

plr = wl.Game().players[1]
map = wl.Game().map

include "data/scripting/coroutine.lua"
include "data/scripting/infrastructure.lua"
include "data/scripting/messages.lua"
include "data/scripting/table.lua"
include "data/scripting/ui.lua"

-- Constants
sf = map.player_slots[1].starting_field
first_lumberjack_field = map:get_field(16, 10)
first_quarry_field = map:get_field(8, 12)
second_quarry_field = map:get_field(5, 10)
road_building_field = map:get_field(9, 12)

-- Global variables
registered_player_immovables = {}
terminate_bad_boy_sentinel = false
immovable_is_legal = function(i) return false end

include "map:scripting/starting_conditions.lua"

-- Starting conditions switch the textdomain
set_textdomain("scenario_tutorial01_basic_control.wmf")

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"
include "map:scripting/helper_functions_demonstration.lua"

include "map:scripting/mission_thread.lua"

