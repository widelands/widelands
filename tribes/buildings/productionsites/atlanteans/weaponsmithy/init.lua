dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_weaponsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Weapon Smithy"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 2,
		planks = 2,
		spidercloth = 1,
		quartz = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1,
		planks = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 50, 58 },
		},
		working = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 50, 58 },
		}
	},

   aihints = {
		prohibited_till = 900
   },

	working_positions = {
		atlanteans_weaponsmith = 1
	},

   inputs = {
		iron = 8,
		gold = 8,
		coal = 8,
		planks = 8
	},
   outputs = {
		"trident_light",
		"trident_long",
		"trident_steel",
		"trident_double",
		"trident_heavy_double"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_trident_light",
				"call=produce_trident_long",
				"call=produce_trident_steel",
				"call=produce_trident_double",
				"call=produce_trident_heavy_double",
				"return=skipped"
			}
		},
		produce_trident_light = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a light trident because ...
			descname = _"forging a light trident",
			actions = {
				"return=skipped unless economy needs trident_light",
				"sleep=20000",
				"consume=iron planks",
				"playFX=sound/smiths smith 192",
				"animate=working 30000",
				"playFX=sound/smiths sharpening 192",
				"produce=trident_light"
			}
		},
		produce_trident_long = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a long trident because ...
			descname = _"forging a long trident",
			actions = {
				"return=skipped unless economy needs trident_long",
				"sleep=32000",
				"consume=iron coal planks",
				"playFX=sound/smiths smith 192",
				"animate=working 45000",
				"playFX=sound/smiths sharpening 192",
				"produce=trident_long"
			}
		},
		produce_trident_steel = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a steel trident because ...
			descname = _"forging a steel trident",
			actions = {
				"return=skipped unless economy needs trident_steel",
				"sleep=32000",
				"consume=iron:2 coal planks",
				"playFX=sound/smiths smith 192",
				"animate=working 45000",
				"playFX=sound/smiths sharpening 192",
				"produce=trident_steel"
			}
		},
		produce_trident_double = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a double trident because ...
			descname = _"forging a double trident",
			actions = {
				"return=skipped unless economy needs trident_double",
				"sleep=32000",
				"consume=iron coal:2 planks gold",
				"playFX=sound/smiths smith 192",
				"animate=working 45000",
				"playFX=sound/smiths sharpening 192",
				"produce=trident_double"
			}
		},
		produce_trident_heavy_double = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a heavy double trident because ...
			descname = _"forging a heavy double trident",
			actions = {
				"return=skipped unless economy needs trident_heavy_double",
				"sleep=32000",
				"consume=iron:2 coal:2 planks gold",
				"playFX=sound/smiths smith 192",
				"animate=working 45000",
				"playFX=sound/smiths sharpening 192",
				"produce=trident_heavy_double"
			}
		},
	},
}
