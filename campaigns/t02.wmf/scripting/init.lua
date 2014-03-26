-- =======================================================================
--                       Barbarians Campaign Mission 2
-- =======================================================================

set_textdomain("scenario_t02.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"

-- ==========
-- Constants
-- ==========
plr = wl.Game().players[1]

-- ===============
-- Initialization
-- ===============
include "map:scripting/starting_conditions.lua"


-- ==============
-- Mission Logic
-- ==============
include "map:scripting/mission_thread.lua"
