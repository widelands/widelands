-- ===============
-- Initialization
-- ===============

plr:allow_buildings("all")
plr:forbid_buildings{"barbarians_market"}

-- A default headquarters
include "tribes/initialization/barbarians/starting_conditions/headquarters.lua"
init.func(plr) -- defined in headquarters

hq_original_name = sf.immovable.warehousename
