-- =======================================================================
--                         Frisian Campaign Mission 1
-- =======================================================================
set_textdomain("scenario_fri01.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1]
p2 = wl.Game().players[2]

firstToFlood = wl.Game().map:get_field(9, 89)
expansionMark = wl.Game().map:get_field(86, 66)

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"

