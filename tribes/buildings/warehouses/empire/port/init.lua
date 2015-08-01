-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Port",
   icon = dirname .. "menu.png",
   size = "port",

   buildcost = {
		log = 3,
		planks = 4,
		granite = 4,
		marble = 2,
		marble_column = 1,
		cloth = 3,
		gold = 2
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 2,
		marble = 2,
		cloth = 1,
		gold = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = no_purpose_text_yet(),
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 74, 96 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 74, 96 },
			fps = 1
		}
	},

	aihints = {
		prohibited_till = 900
	},

   conquers = 5,
   heal_per_second = 170,
}
