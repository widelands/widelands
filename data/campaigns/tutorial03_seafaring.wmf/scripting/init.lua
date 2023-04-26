-- =======================================================================
--                          Tutorial 03: Seafaring
-- =======================================================================

plr = wl.Game().players[1]
map = wl.Game().map

push_textdomain("scenario_tutorial03_seafaring.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

wl.Game().allow_diplomacy = false
sf = map.player_slots[1].starting_field
second_port_field = map:get_field(37, 27)
shipyard_tip = map:get_field(37, 41)
port_on_island = map:get_field(102, 36)
iron_on_island = map:get_field(97,35)
additional_port_space = map:get_field(85, 5)
castle_field = map:get_field(36, 20)
waterway_field = map:get_field(27, 95)
gold_mine = map:get_field(23, 102)
shore = map:get_field(34, 68)
home_bay = map:get_field(39, 34)

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"
include "map:scripting/helper_functions_demonstration.lua"

swimmable_fields = get_swimmable_fields()

include "map:scripting/starting_conditions.lua"

include "map:scripting/mission_thread.lua"
