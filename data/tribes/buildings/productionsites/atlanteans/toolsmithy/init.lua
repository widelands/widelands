push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_toolsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Toolsmithy"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 56, 61 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 56, 61 },
      }
   },

   aihints = {
   },

   working_positions = {
      atlanteans_toolsmith = 1
   },

   inputs = {
      { name = "log", amount = 6 },
      { name = "spidercloth", amount = 4 },
      { name = "iron", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_fire_tongs",
            "call=produce_hunting_bow",
            "call=produce_pick",
            "call=produce_hammer",
            "call=produce_saw",
            "call=produce_shovel",
            "call=produce_scythe",
            "call=produce_bread_paddle",
            "call=produce_hook_pole",
            "call=produce_buckets",
            "call=produce_milking_tongs",
            "call=produce_fishing_net",
         }
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _("making a bread paddle"),
         actions = {
            "return=skipped unless economy needs bread_paddle",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=bread_paddle"
         }
      },
      produce_buckets = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pair of buckets because ...
         descname = _("making a pair of buckets"),
         actions = {
            "return=skipped unless economy needs buckets",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=buckets"
         }
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _("making fire tongs"),
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=fire_tongs"
         }
      },
      produce_fishing_net = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _("making a fishing net"),
         actions = {
            "return=skipped unless economy needs fishing_net",
            "consume=spidercloth:2",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=fishing_net"
         }
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _("making a hammer"),
         actions = {
            "return=skipped unless economy needs hammer",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=hammer"
         }
      },
      produce_hook_pole = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hook pole because ...
         descname = _("making a hook pole"),
         actions = {
            "return=skipped unless economy needs hook_pole",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=hook_pole"
         }
      },
      produce_hunting_bow = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting bow because ...
         descname = _("making a hunting bow"),
         actions = {
            "return=skipped unless economy needs hunting_bow",
            "consume=log spidercloth",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=hunting_bow"
         }
      },
      produce_milking_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making milking tongs because ...
         descname = _("making milking tongs"),
         actions = {
            "return=skipped unless economy needs milking_tongs",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=milking_tongs"
         }
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _("making a pick"),
         actions = {
            "return=skipped unless economy needs pick",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=pick"
         }
      },
      produce_saw = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a saw because ...
         descname = _("making a saw"),
         actions = {
            "return=skipped unless economy needs saw",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=saw"
         }
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _("making a scythe"),
         actions = {
            "return=skipped unless economy needs scythe",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=scythe"
         }
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _("making a shovel"),
         actions = {
            "return=skipped unless economy needs shovel",
            "consume=iron log",
            "sleep=duration:32s",
            "playsound=sound/smiths/toolsmith priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=shovel"
         }
      },
   },
}

pop_textdomain()
