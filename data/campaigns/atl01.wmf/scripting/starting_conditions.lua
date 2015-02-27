-- =======================================================================
--                      Atlanteans Tutorial Mission 01
-- =======================================================================

-- ===============
-- Initialization
-- ===============

p1:allow_buildings("all")
p1:forbid_buildings{"shipyard"}

-- A default headquarters
include "tribes/atlanteans/scripting/sc00_headquarters.lua"
init.func(p1) -- defined in sc00_headquarters

local hq = wl.Game().map.player_slots[1].starting_field.immovable
hq:set_workers{shipwright=1}

-- Place some buildings
prefilled_buildings(p1,
	{"high_tower", first_tower_field.x, first_tower_field.y,
		soldiers = { [{0,0,0,0}] = 1 }
	},
	{"high_tower", second_tower_field.x, second_tower_field.y,
		soldiers = { [{0,0,0,0}] = 1 }
	}
)
