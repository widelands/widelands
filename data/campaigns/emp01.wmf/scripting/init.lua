-- =======================================================================
--                         Empire Campaign Mission 1
-- =======================================================================
push_textdomain("scenario_emp01.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/lock.lua"

p1 = wl.Game().players[1]

include "map:scripting/helper_functions.lua"
include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
