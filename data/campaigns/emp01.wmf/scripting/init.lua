-- =======================================================================
--                         Empire Campaign Mission 1
-- =======================================================================
set_textdomain("scenario_emp01.wmf")

include "data/scripting/coroutine.lua"
include "data/scripting/objective_utils.lua"
include "data/scripting/infrastructure.lua"
include "data/scripting/table.lua"
include "data/scripting/ui.lua"

p1 = wl.Game().players[1]

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
