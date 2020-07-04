dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_metal_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Metal Workshop"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_ax_workshop",

   buildcost = {
      log = 1,
      blackwood = 1,
      granite = 2,
      grout = 1,
      reed = 1
   },
   return_on_dismantle = {
      blackwood = 1,
      granite = 1,
      grout = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 76 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 57, 76 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 57, 76 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 76 },
         fps = 10
      },
   },

   aihints = {
      basic_amount = 2,
      prohibited_till = 400,
      weak_ai_limit = 2,
      very_weak_ai_limit = 1
   },

   working_positions = {
      barbarians_blacksmith = 1
   },

   inputs = {
      { name = "log", amount = 8 },
      { name = "iron", amount = 8 }
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
            -- firetongs before felling ax to make poor hamlet work without dismantle
            "call=produce_fire_tongs",
            "call=produce_felling_ax",
            "call=produce_fishing_rod",
            "call=produce_hammer",
            "call=produce_hunting_spear",
            "call=produce_kitchen_tools",
            "call=produce_pick",
            "call=produce_scythe",
            "call=produce_shovel",
         }
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _"making a bread paddle",
         actions = {
            "return=skipped unless economy needs bread_paddle",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=bread_paddle"
         }
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
         descname = _"making a felling ax",
         actions = {
            "return=skipped unless economy needs felling_ax",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=felling_ax"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _"making fire tongs",
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=fire_tongs"
         }
      },
      produce_fishing_rod = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing rod because ...
         descname = _"making a fishing rod",
         actions = {
            "return=skipped unless economy needs fishing_rod",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=fishing_rod"
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _"making a hammer",
         actions = {
            "return=skipped unless economy needs hammer",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=hammer"
         }
      },
      produce_hunting_spear = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _"making a hunting spear",
         actions = {
            "return=skipped unless economy needs hunting_spear",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=hunting_spear"
         }
      },
      produce_kitchen_tools = {
         -- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
         descname = _"making kitchen tools",
         actions = {
            "return=skipped unless economy needs kitchen_tools",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=kitchen_tools"
         }
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _"making a pick",
         actions = {
            "return=skipped unless economy needs pick",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=pick"
         }
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _"making a scythe",
         actions = {
            "return=skipped unless economy needs scythe",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=scythe"
         }
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _"making a shovel",
         actions = {
            "return=skipped unless economy needs shovel",
            "consume=iron log",
            "sleep=32000",
            "playsound=sound/smiths/toolsmith 192",
            "animate=working 35000",
            "produce=shovel"
         }
      },
   },
}
