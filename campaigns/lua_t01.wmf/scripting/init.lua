-- =======================================================================
--                       Barbarians Campaign Mission 1                      
-- =======================================================================

-- ===============
-- Initialization 
-- ===============

-- Only lumberjack buildings are allowed
p = wl.map.Player(1)
p:forbid_buildings("all")
p:allow_buildings{"lumberjacks_hut"}


