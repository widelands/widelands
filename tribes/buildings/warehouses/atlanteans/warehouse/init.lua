-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "atlanteans_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Warehouse",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 2,
		quartz = 1,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 1,
		quartz = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Warehouses store soldiers, wares and tools.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 58, 62 }
		}
	},

	aihints = {},

   heal_per_second = 170,
}
