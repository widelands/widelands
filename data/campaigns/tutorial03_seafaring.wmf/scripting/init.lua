-- =======================================================================
--                          Tutorial 03: Seafaring
-- =======================================================================

plr = wl.Game().players[1]
map = wl.Game().map

set_textdomain("scenario_tutorial03_seafaring.wmf")

include "data/scripting/coroutine.lua"
include "data/scripting/infrastructure.lua"
include "data/scripting/messages.lua"
include "data/scripting/table.lua"
include "data/scripting/ui.lua"

sf = map.player_slots[1].starting_field
second_port_field = map:get_field(37, 27)
port_on_island = map:get_field(102, 36)
additional_port_space = map:get_field(85, 5)
castle_field = map:get_field(36, 20)

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"

include "map:scripting/starting_conditions.lua"

include "map:scripting/mission_thread.lua"

