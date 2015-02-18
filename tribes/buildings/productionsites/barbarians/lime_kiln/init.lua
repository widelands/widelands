dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_lime_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Lime Kiln",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 4,
		granite = 2,
		blackwood = 1
	},
	return_on_dismantle = {
		log = 2,
		granite = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = _"‘Forming new stone from old with fire and water.’",
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = _"Ragnald the Child’s answer to the question, what he’s doing in the dirt. His ‘new stone’ now is an important building material.",
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = _"The lime kiln processes granite to make ‘grout’, a substance that solidifies and so reinforces masonry.",
		-- #TRANSLATORS: Note helptext for a building
		note = _"The lime kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, this building can produce grout in about %s on average.":bformat(ngettext("%d second", "%d seconds", 41):bformat(41))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 45, 53 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 45, 53 },
		},
	},

	working_positions = {
		barbarians_lime_burner = 1
	},

   inputs = {
		granite = 6,
		water = 6,
		coal = 3
	},
   outputs = {
		"grout"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mixing grout because ...
			descname = _"mixing grout",
			actions = {
				"sleep=50000",
				"return=skipped unless economy needs grout",
				"consume=coal granite:2 water:2",
				"animate=working 32000",
				"produce=grout:2"
			}
		},
	},
}
