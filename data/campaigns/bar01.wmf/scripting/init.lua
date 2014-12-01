-- =======================================================================
--                       Barbarians Campaign Mission 1
-- =======================================================================

set_textdomain("scenario_bar01.wmf")

include "data/scripting/coroutine.lua"
include "data/scripting/objective_utils.lua"
include "data/scripting/infrastructure.lua"
include "data/scripting/messages.lua"

-- ==========
-- Constants
-- ==========
plr = wl.Game().players[1]
map = wl.Game().map

sf = map:get_field(12,10)
al_thunran = map:get_field(5,46)
grave = map:get_field(24,28)

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"

include "map:scripting/starting_conditions.lua"

include "map:scripting/mission_thread.lua"

