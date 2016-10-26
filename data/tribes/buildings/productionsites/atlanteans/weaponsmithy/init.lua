dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_weaponsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Weapon Smithy"),
   helptext_script = dirname .. "helptexts.lua",
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
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
      { name = "planks", amount = 8 },
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 }
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
            "play_sound=sound/smiths smith 192",
            "animate=working 21000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=trident_light"
         }
      },
      produce_trident_long = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a long trident because ...
         descname = _"forging a long trident",
         actions = {
            "return=skipped unless economy needs trident_long",
            "consume=iron coal planks",
            "sleep=32000",
            "play_sound=sound/smiths smith 192",
            "animate=working 36000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=trident_long"
         }
      },
      produce_trident_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel trident because ...
         descname = _"forging a steel trident",
         actions = {
            "return=skipped unless economy needs trident_steel",
            "consume=iron:2 coal planks",
            "sleep=32000",
            "play_sound=sound/smiths smith 192",
            "animate=working 36000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=trident_steel"
         }
      },
      produce_trident_double = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a double trident because ...
         descname = _"forging a double trident",
         actions = {
            "return=skipped unless economy needs trident_double",
            "consume=iron coal:2 planks gold",
            "sleep=32000",
            "play_sound=sound/smiths smith 192",
            "animate=working 36000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=trident_double"
         }
      },
      produce_trident_heavy_double = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a heavy double trident because ...
         descname = _"forging a heavy double trident",
         actions = {
            "return=skipped unless economy needs trident_heavy_double",
            "consume=iron:2 coal:2 planks gold",
            "sleep=32000",
            "play_sound=sound/smiths smith 192",
            "animate=working 36000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=trident_heavy_double"
         }
      },
   },
}
