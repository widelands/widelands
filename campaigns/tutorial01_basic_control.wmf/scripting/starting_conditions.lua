-- ===============
-- Initialization
-- ===============

plr:allow_buildings("all")

-- A default headquarters
include "tribes/scripting/starting_conditions/barbarians/sc00_headquarters.lua"
init.func(plr) -- defined in sc00_headquarters

