dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_toolsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Toolsmithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      granite = 2,
      planks = 2,
      spidercloth = 1
   },
   return_on_dismantle = {
      granite = 1,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 53 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 50, 53 },
      }
   },

   aihints = {
      prohibited_till = 450
   },

   working_positions = {
      atlanteans_toolsmith = 1
   },

   inputs = {
      { name = "log", amount = 6 },
      { name = "spidercloth", amount = 4 },
      { name = "iron", amount = 6 }
   },
   outputs = {
      "bread_paddle",
      "buckets",
      "fire_tongs",
      "fishing_net",
      "hammer",
      "hook_pole",
      "hunting_bow",
      "milking_tongs",
      "pick",
      "saw",
      "scythe",
      "shovel"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_bread_paddle",
            "call=produce_buckets",
            "call=produce_fire_tongs",
            "call=produce_fishing_net",
            "call=produce_hammer",
            "call=produce_hook_pole",
            "call=produce_hunting_bow",
            "call=produce_milking_tongs",
            "call=produce_pick",
            "call=produce_saw",
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
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=bread_paddle"
         }
      },
      produce_buckets = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pair of buckets because ...
         descname = _"making a pair of buckets",
         actions = {
            "return=skipped unless economy needs buckets",
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=buckets"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _"making fire tongs",
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=fire_tongs"
         }
      },
      produce_fishing_net = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _"making a fishing net",
         actions = {
            "return=skipped unless economy needs fishing_net",
            "sleep=5000",
            "consume=spidercloth:2",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=fishing_net"
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _"making a hammer",
         actions = {
            "return=skipped unless economy needs hammer",
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=hammer"
         }
      },
      produce_hook_pole = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hook pole because ...
         descname = _"making a hook pole",
         actions = {
            "return=skipped unless economy needs hook_pole",
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=hook_pole"
         }
      },
      produce_hunting_bow = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting bow because ...
         descname = _"making a hunting bow",
         actions = {
            "return=skipped unless economy needs hunting_bow",
            "sleep=5000",
            "consume=log spidercloth",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=hunting_bow"
         }
      },
      produce_milking_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making milking tongs because ...
         descname = _"making milking tongs",
         actions = {
            "return=skipped unless economy needs milking_tongs",
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=milking_tongs"
         }
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _"making a pick",
         actions = {
            "return=skipped unless economy needs pick",
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
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
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
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
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
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
            "sleep=5000",
            "consume=iron log",
            "sleep=27000",
            "play_sound=sound/smiths toolsmith 192",
            "animate=working 35000",
            "produce=shovel"
         }
      },
   },
}
