dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_metal_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Metal Workshop"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_axfactory",

   buildcost = {
		log = 1,
		blackwood = 1,
		granite = 2,
		grout = 1,
		thatch_reed = 1
	},
	return_on_dismantle = {
		blackwood = 1,
		granite = 1,
		grout = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 57, 76 },
		},
		build = {
			template = "build_??",
			directory = dirname,
			hotspot = { 57, 76 },
		},
		unoccupied = {
			template = "unoccupied_??",
			directory = dirname,
			hotspot = { 57, 76 },
		},
		working = {
			template = "working_??",
			directory = dirname,
			hotspot = { 57, 76 },
			fps = 10
		},
	},

   aihints = {
		forced_after = 300,
		prohibited_till = 120
   },

	working_positions = {
		barbarians_blacksmith = 1
	},

   inputs = {
		iron = 8,
		log = 8
	},
   outputs = {
		"bread_paddle",
		"felling_ax",
		"fire_tongs",
		"fishing_rod",
		"hammer",
		"hunting_spear",
		"kitchen_tools",
		"pick",
		"scythe",
		"shovel"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_bread_paddle",
				"call=produce_felling_ax",
				"call=produce_fire_tongs",
				"call=produce_fishing_rod",
				"call=produce_hammer",
				"call=produce_hunting_spear",
				"call=produce_kitchen_tools",
				"call=produce_pick",
				"call=produce_scythe",
				"call=produce_shovel",
				"return=skipped"
			}
		},
		produce_bread_paddle = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
			descname = _"making a bread paddle",
			actions = {
				"return=skipped unless economy needs bread_paddle",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=bread_paddle"
			}
		},
		produce_felling_ax = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
			descname = _"making a felling ax",
			actions = {
				"return=skipped unless economy needs felling_ax",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=felling_ax"
			}
		},
		produce_fire_tongs = {
			-- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
			descname = _"making fire tongs",
			actions = {
				"return=skipped unless economy needs fire_tongs",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=fire_tongs"
			}
		},
		produce_fishing_rod = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a fishing rod because ...
			descname = _"making a fishing rod",
			actions = {
				"return=skipped unless economy needs fishing_rod",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=fishing_rod"
			}
		},
		produce_hammer = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
			descname = _"making a hammer",
			actions = {
				"return=skipped unless economy needs hammer",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=hammer"
			}
		},
		produce_hunting_spear = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
			descname = _"making a hunting spear",
			actions = {
				"return=skipped unless economy needs hunting_spear",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=hunting_spear"
			}
		},
		produce_kitchen_tools = {
			-- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
			descname = _"making kitchen tools",
			actions = {
				"return=skipped unless economy needs kitchen_tools",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=kitchen_tools"
			}
		},
		produce_pick = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
			descname = _"making a pick",
			actions = {
				"return=skipped unless economy needs pick",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=pick"
			}
		},
		produce_scythe = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
			descname = _"making a scythe",
			actions = {
				"return=skipped unless economy needs scythe",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=scythe"
			}
		},
		produce_shovel = {
			-- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
			descname = _"making a shovel",
			actions = {
				"return=skipped unless economy needs shovel",
				"sleep=32000",
				"consume=iron log",
				"playFX=sound/smiths toolsmith 192",
				"animate=working 35000",
				"produce=shovel"
			}
		},
	},
}
