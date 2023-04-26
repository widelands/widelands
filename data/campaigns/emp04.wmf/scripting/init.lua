-- =======================================================================
--                         Empire Campaign Mission 4
-- =======================================================================

push_textdomain("scenario_emp04.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

include "map:scripting/helper_functions.lua"

wl.Game().allow_diplomacy = false
p1 = wl.Game().players[1]
p2 = wl.Game().players[2]
p3 = wl.Game().players[3]

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"

map = wl.Game().map
mv = wl.ui.MapView()
sf = map.player_slots[1].starting_field
