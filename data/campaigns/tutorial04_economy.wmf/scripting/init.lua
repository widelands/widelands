-- =======================================================================
--                          Tutorial 04: Economy
-- =======================================================================

plr = wl.Game().players[1]
plr2 = wl.Game().players[2]

set_textdomain("scenario_tutorial04_economy.wmf")

include "data/scripting/coroutine.lua"
include "data/scripting/infrastructure.lua"
include "data/scripting/messages.lua"
include "data/scripting/table.lua"
include "data/scripting/ui.lua"

map = wl.Game().map
mv = wl.ui.MapView()

tavern_field = map:get_field(105, 44)
warehouse_field = map:get_field(78, 67)
field_near_border = map:get_field(69, 66)

sf = map.player_slots[1].starting_field

include "map:scripting/helper_functions.lua"
include "map:scripting/texts.lua"

include "map:scripting/starting_conditions.lua"

include "map:scripting/mission_thread.lua"

