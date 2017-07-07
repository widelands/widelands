dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_blacksmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Blacksmithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 6,
      granite = 3,
      log = 3,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 3,
      granite = 2,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 76 },
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
      },
   },

   aihints = {
      prohibited_till = 400,
      forced_after = 400
   },

   working_positions = {
      frisians_blacksmith = 1
   },

   inputs = {
      { name = "log", amount = 8 },
      { name = "iron", amount = 8 },
   },
   outputs = {
      "felling_ax",
      "pick",
      "scythe",
      "shovel",
      "basket",
      "hunting_spear",
      "fishing_net",
      "bread_paddle",
      "kitchen_tools",
      "hammer",
      "fire_tongs",
      "needles",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"working",
         actions = {
            "call=produce_1",
            "call=produce_2",
            "call=produce_3",
            "call=produce_4",
            "call=produce_5",
            "call=produce_6",
            "call=produce_7",
            "call=produce_8",
            "call=produce_9",
            "call=produce_10",
            "call=produce_11",
            "call=produce_12",
            "return=skipped",
         },
      },
      produce_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a felling ax",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs felling_ax",
            "consume=log iron",
            "animate=working 19000",
            "produce=felling_ax"
         },
      },
      produce_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a pick",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs pick",
            "consume=log iron",
            "animate=working 19000",
            "produce=pick"
         },
      },
      produce_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a scythe",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs scythe",
            "consume=log iron",
            "animate=working 19000",
            "produce=scythe"
         },
      },
      produce_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a shovel",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs shovel",
            "consume=log iron",
            "animate=working 19000",
            "produce=shovel"
         },
      },
      produce_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a basket",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs basket",
            "consume=log iron",
            "animate=working 19000",
            "produce=basket"
         },
      },
      produce_6 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a hunting spear",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs hunting_spear",
            "consume=log iron",
            "animate=working 19000",
            "produce=hunting_spear"
         },
      },
      produce_7 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a fishing net",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs fishing_net",
            "consume=log iron",
            "animate=working 19000",
            "produce=fishing_net"
         },
      },
      produce_8 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a bread paddle",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs bread_paddle",
            "consume=log iron",
            "animate=working 19000",
            "produce=bread_paddle"
         },
      },
      produce_9 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging kitchen tools",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs kitchen_tools",
            "consume=log iron",
            "animate=working 19000",
            "produce=kitchen_tools"
         },
      },
      produce_10 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging a hammer",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs hammer",
            "consume=log iron",
            "animate=working 19000",
            "produce=hammer"
         },
      },
      produce_11 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging fire tongs",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs fire_tongs",
            "consume=log iron",
            "animate=working 19000",
            "produce=fire_tongs"
         },
      },
      produce_12 = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"forging needles",
         actions = {
            "sleep=12000",
            "return=skipped unless economy needs needles",
            "consume=log iron",
            "animate=working 19000",
            "produce=needles"
         },
      },
   },
}
