-- ===============
-- Initialization
-- ===============

plr:allow_buildings("all")

-- A default headquarters
include "tribes/scripting/starting_conditions/barbarians/headquarters.lua"
init.func(plr) -- defined in headquarters
