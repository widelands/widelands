-- =======================================================================
--                         Empire Campaign Mission 3
-- =======================================================================
set_textdomain("scenario_emp03.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"


p1 = wl.Game().players[1]
p2 = wl.Game().players[2]

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"

map = wl.Game().map
mv = wl.ui.MapView()
sf = map.player_slots[1].starting_field
