push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smithy_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Smithy"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "europeans_smithy_level_1",
      enhancement_cost = {
         planks = 1,
         brick = 1,
         grout = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         log = 1,
         granite = 1
      }
   },

   buildcost = {
      planks = 3,
      reed = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 1
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
      basic_amount = 1
   },

   working_positions = {
      europeans_smith_basic = 1
   },

   inputs = {
      { name = "planks", amount = 4 },
      { name = "iron", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_hammer",
            "call=produce_saw",
            "call=produce_felling_ax",
            "call=produce_pick",
            "call=produce_shovel",
            "call=produce_buckets",
            "call=produce_fire_tongs",
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _"making a hammer",
         actions = {
            "return=skipped unless economy needs hammer",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=hammer"
         }
      },
      produce_saw = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a saw because ...
         descname = _"making a saw",
         actions = {
            "return=skipped unless economy needs saw",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=saw"
         }
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
         descname = _"making a felling ax",
         actions = {
            "return=skipped unless economy needs felling_ax",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=felling_ax"
         }
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _"making a pick",
         actions = {
            "return=skipped unless economy needs pick",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=pick"
         }
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _"making a shovel",
         actions = {
            "return=skipped unless economy needs shovel",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=shovel"
         }
      },
      produce_buckets = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pair of buckets because ...
         descname = _"making a pair of buckets",
         actions = {
            "return=skipped unless economy needs buckets",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=buckets"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _"making fire tongs",
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "consume=planks iron",
            "sleep=duration:60s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=fire_tongs"
         }
      },
   },
}

pop_textdomain()
