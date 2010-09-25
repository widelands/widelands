-- =======================================================================
--                       Barbarians Campaign Mission 2                      
-- =======================================================================

set_textdomain("scenario_t02.wmf")

use("aux", "coroutine")
use("aux", "objective_utils")
use("aux", "infrastructure")

-- ==========
-- Constants 
-- ==========
plr = wl.Game().players[1]

-- ===============
-- Initialization 
-- ===============
use("map", "starting_conditions")


-- ==============
-- Mission Logic 
-- ==============
use("map", "mission_thread")

