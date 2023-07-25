push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_weaponsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Weapon Smithy"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 56, 67 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 56, 67 },
      }
   },

   aihints = {
      prohibited_till = 1400
   },

   working_positions = {
      atlanteans_weaponsmith = 1
   },

   inputs = {
      { name = "planks", amount = 8 },
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_trident_light",
            "call=produce_trident_long",
            "call=produce_trident_steel",
            "call=produce_trident_double",
            "call=produce_trident_heavy_double",
         }
      },
      produce_trident_light = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a light trident because ...
         descname = _("forging a light trident"),
         actions = {
            -- time total: 50 + 3.6
            "return=skipped unless economy needs trident_light",
            "consume=iron planks",
            "sleep=duration:20s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:21s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=trident_light"
         }
      },
      produce_trident_long = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a long trident because ...
         descname = _("forging a long trident"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs trident_long",
            "consume=iron coal planks",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:36s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=trident_long"
         }
      },
      produce_trident_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel trident because ...
         descname = _("forging a steel trident"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs trident_steel",
            "consume=iron:2 coal planks",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:36s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=trident_steel"
         }
      },
      produce_trident_double = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a double trident because ...
         descname = _("forging a double trident"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs trident_double",
            "consume=iron coal:2 planks gold",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:36s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=trident_double"
         }
      },
      produce_trident_heavy_double = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a heavy double trident because ...
         descname = _("forging a heavy double trident"),
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs trident_heavy_double",
            "consume=iron:2 coal:2 planks gold",
            "sleep=duration:32s",
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:36s",
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=trident_heavy_double"
         }
      },
   },
}

pop_textdomain()
