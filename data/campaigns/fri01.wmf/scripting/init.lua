-- =======================================================================
--                         Frisian Campaign Mission 1
-- =======================================================================
set_textdomain("scenario_fri01.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

map = wl.Game().map
p1 = wl.Game().players[1]
p2 = wl.Game().players[2]

first_to_flood = map:get_field(9, 89)
expansion_mark = map:get_field(68, 68)
warehouse_mark = map:get_field(44, 77)
port_space = map:get_field(210, 10)

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"

