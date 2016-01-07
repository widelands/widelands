-- =======================================================================
--                         Empire Campaign Mission 1
-- =======================================================================
set_textdomain("scenario_emp01.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1]

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
