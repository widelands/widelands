-- =======================================================================
--                          Tutorial 04: Economy
-- =======================================================================

plr = wl.Game().players[1]
plr2 = wl.Game().players[2]

push_textdomain("scenario_tutorial04_economy.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

map = wl.Game().map
mv = wl.ui.MapView()
wl.Game().allow_diplomacy = false

tavern_field = map:get_field(105, 44)
warehouse_field = map:get_field(78, 67)
field_near_border = map:get_field(69, 66)

sf = map.player_slots[1].starting_field

include "map:scripting/helper_functions.lua"
include "map:scripting/texts.lua"

include "map:scripting/starting_conditions.lua"

include "map:scripting/mission_thread.lua"
