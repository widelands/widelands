push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_metal_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Metal Workshop"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "barbarians_ax_workshop",
      enhancement_cost = {
         log = 1,
         blackwood = 1,
         granite = 2,
         grout = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         blackwood = 1,
         granite = 1,
         grout = 1
      }
   },

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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 57, 76 },
      },
      unoccupied = {
         hotspot = { 57, 76 },
      },
   },
   spritesheets = {
      working = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 57, 64 }
      },
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 57, 64 }
      }
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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- "return=skipped" causes 10 sec delay
            -- time total: 10 * 70 + 10 = 710 sec
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
            "return=skipped"
         }
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _("making a bread paddle"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs bread_paddle",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=bread_paddle"
         }
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
         descname = _("making a felling ax"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs felling_ax",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=felling_ax"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _("making fire tongs"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs fire_tongs",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=fire_tongs"
         }
      },
      produce_fishing_rod = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing rod because ...
         descname = _("making a fishing rod"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs fishing_rod",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=fishing_rod"
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _("making a hammer"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs hammer",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=hammer"
         }
      },
      produce_hunting_spear = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _("making a hunting spear"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs hunting_spear",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=hunting_spear"
         }
      },
      produce_kitchen_tools = {
         -- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
         descname = _("making kitchen tools"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs kitchen_tools",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=kitchen_tools"
         }
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _("making a pick"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs pick",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=pick"
         }
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _("making a scythe"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs scythe",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=scythe"
         }
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _("making a shovel"),
         actions = {
            -- time: 31.4 + 35 + 3.6 = 70 sec
            "return=skipped unless economy needs shovel",
            "consume=iron log",
            "sleep=duration:31s400ms",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=shovel"
         }
      },
   },
}

pop_textdomain()
