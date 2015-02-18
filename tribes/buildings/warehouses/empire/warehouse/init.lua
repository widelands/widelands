dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Warehouse",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 2,
		marble = 3,
		marble_column = 2
	},
	return_on_dismantle = {
		planks = 1,
		granite = 2,
		marble = 2,
		marble_column = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Warehouses store soldiers, wares and tools.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 58, 55 }
		}
	},

   heal_per_second = 170,
}
