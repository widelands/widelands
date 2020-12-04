push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smithy_level_2",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Smithy Level 2"),
   icon = dirname .. "menu.png",
   size = "medium",
   
   enhancement = {
      name = "europeans_smithy_level_3",
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
   },

   working_positions = {
      europeans_smith_basic = 2
   },

   inputs = {
      { name = "planks", amount = 6 },
      { name = "iron", amount = 6 },
      { name = "spidercloth", amount = 4 }
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
            "call=produce_scythe",
            "call=produce_basket",
            "call=produce_milking_tongs",
            "call=produce_needles",
            "call=produce_bread_paddle",
            "call=produce_fishing_rod",
            "call=produce_fishing_net",
            "call=produce_hunting_bow",
            "call=produce_hunting_spear",
            "call=produce_hook_pole"
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _"making a hammer",
         actions = {
            "return=skipped unless economy needs hammer",
            "consume=planks iron",
            "sleep=duration:40s",
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
            "sleep=duration:40s",
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
            "sleep=duration:40s",
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
            "sleep=duration:40s",
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
            "sleep=duration:40s",
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
            "sleep=duration:40s",
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
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=fire_tongs"
         }
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _"making a scythe",
         actions = {
            "return=skipped unless economy needs scythe",
            "consume=planks iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=scythe"
         }
      },
      produce_basket = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a basket because ...
         descname = _"making a basket",
         actions = {
            "return=skipped unless economy needs basket",
            "consume=planks iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=basket"
         }
      },
      produce_milking_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making milking tongs because ...
         descname = _"making milking tongs",
         actions = {
            "return=skipped unless economy needs milking_tongs",
            "consume=planks iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=milking_tongs"
         }
      },
      produce_needles = {
         -- TRANSLATORS: Completed/Skipped/Did not start making milking tongs because ...
         descname = _"making needles",
         actions = {
            "return=skipped unless economy needs milking_tongs",
            "consume=iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=needles"
         }
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _"making a bread paddle",
         actions = {
            "return=skipped unless economy needs bread_paddle",
            "consume=planks iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=bread_paddle"
         }
      },
      produce_fishing_rod = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing rod because ...
         descname = _"making a fishing rod",
         actions = {
            "return=skipped unless economy needs fishing_rod",
            "consume=planks spidercloth",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=fishing_rod"
         }
      },
      produce_fishing_net = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _"making a fishing net",
         actions = {
            "return=skipped unless economy needs fishing_net",
            "consume=spidercloth:2",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=fishing_net"
         }
      },
      produce_hunting_bow = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting bow because ...
         descname = _"making a hunting bow",
         actions = {
            "return=skipped unless economy needs hunting_bow",
            "consume=planks spidercloth",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=hunting_bow"
         }
      },
      produce_hunting_spear = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _"making a hunting spear",
         actions = {
            "return=skipped unless economy needs hunting_spear",
            "consume=planks iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=hunting_spear"
         }
      },
      produce_hook_pole = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hook pole because ...
         descname = _"making a hook pole",
         actions = {
            "return=skipped unless economy needs hook_pole",
            "consume=planks iron",
            "sleep=duration:40s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:60s",
            "produce=hook_pole"
         }
      }
   },
}

pop_textdomain()
