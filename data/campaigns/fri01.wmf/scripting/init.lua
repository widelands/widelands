-- =======================================================================
--                         Frisian Campaign Mission 1
-- =======================================================================
push_textdomain("scenario_fri01.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

map = wl.Game().map
p1 = wl.Game().players[1]
p2 = wl.Game().players[2]
difficulty = wl.Game().scenario_difficulty
wl.Game().allow_diplomacy = false

first_to_flood = map:get_field(9, 89)
expansion_mark = map:get_field(68, 68)
backup_mark = map:get_field(187, 36)
warehouse_mark = map:get_field(44, 77)

-- Time in ms that elapses between the drowning of two fields when flooding
flood_speed = {500, 430, 400}
flood_speed = flood_speed[difficulty]

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
