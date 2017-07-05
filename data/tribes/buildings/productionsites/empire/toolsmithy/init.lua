dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_toolsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Toolsmithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      granite = 2,
      marble = 2,
      marble_column = 1
   },
   return_on_dismantle = {
      granite = 1,
      marble = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 61 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 58, 61 },
      },
   },

   aihints = {
      prohibited_till = 400
   },

   working_positions = {
      empire_toolsmith = 1
   },

   inputs = {
      { name = "log", amount = 8 },
      { name = "iron", amount = 8 },
   },
   outputs = {
      "felling_ax",
      "bread_paddle",
      "fire_tongs",
      "fishing_rod",
      "hammer",
      "kitchen_tools",
      "pick",
      "scythe",
      "shovel",
      "hunting_spear",
      "basket",
      "saw"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_felling_ax",
            "call=produce_basket",
            "call=produce_bread_paddle",
            "call=produce_fire_tongs",
            "call=produce_fishing_rod",
            "call=produce_hammer",
            "call=produce_hunting_spear",
            "call=produce_kitchen_tools",
            "call=produce_pick",
            "call=produce_saw",
            "call=produce_scythe",
            "call=produce_shovel",
            "return=skipped"
         }
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
         descname = _"making a felling ax",
         actions = {
            "return=skipped unless economy needs felling_ax",
            "sleep=32000",
            "consume=iron log",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=felling_ax"
         }
      },
      produce_basket = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a basket because ...
         descname = _"making a basket",
         actions = {
            "return=skipped unless economy needs basket",
            "sleep=32000",
            "consume=iron log",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=basket"
         }
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _"making a bread paddle",
         actions = {
            "return=skipped unless economy needs bread_paddle",
            "sleep=32000",
            "consume=iron log",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=bread_paddle"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _"making fire tongs",
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "sleep=32000",
            "consume=iron log",
            "play_sound=sound/smiths toolsmith 192",
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
            "play_sound=sound/smiths toolsmith 192",
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
            "play_sound=sound/smiths toolsmith 192",
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
            "play_sound=sound/smiths toolsmith 192",
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
            "play_sound=sound/smiths toolsmith 192",
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
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=pick"
         }
      },
      produce_saw = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a saw because ...
         descname = _"making a saw",
         actions = {
            "return=skipped unless economy needs saw",
            "sleep=32000",
            "consume=iron log",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=saw"
         }
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _"making a scythe",
         actions = {
            "return=skipped unless economy needs scythe",
            "sleep=32000",
            "consume=iron log",
            "play_sound=sound/smiths toolsmith 192",
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
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=shovel"
         }
      },
   },
}
