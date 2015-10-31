-- =======================================================================
--                      Atlanteans Tutorial Mission 01
-- =======================================================================

-- ===============
-- Initialization
-- ===============

p1:allow_buildings("all")
p1:forbid_buildings{"atlanteans_shipyard"}

-- A default headquarters
include "tribes/scripting/starting_conditions/atlanteans/headquarters.lua"
init.func(p1) -- defined in headquarters

local hq = wl.Game().map.player_slots[1].starting_field.immovable
hq:set_workers{atlanteans_shipwright=1}

-- Place some buildings
prefilled_buildings(p1,
   {"atlanteans_tower_high", first_tower_field.x, first_tower_field.y,
      soldiers = { [{0,0,0,0}] = 1 }
   },
   {"atlanteans_tower_high", second_tower_field.x, second_tower_field.y,
      soldiers = { [{0,0,0,0}] = 1 }
   }
)