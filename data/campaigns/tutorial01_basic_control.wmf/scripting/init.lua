-- =======================================================================
--                       Tutorial 01: Basic Control
-- =======================================================================

plr = wl.Game().players[1]
map = wl.Game().map

push_textdomain("scenario_tutorial01_basic_control.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

wl.Game().allow_diplomacy = false

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

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"
include "map:scripting/helper_functions_demonstration.lua"

include "map:scripting/mission_thread.lua"
