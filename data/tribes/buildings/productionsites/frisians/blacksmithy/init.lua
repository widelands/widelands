push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_blacksmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Blacksmithy"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {50, 73},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      build = {
         hotspot = {50, 58},
         frames = 3,
         columns = 3,
         rows = 1,
      },
      working = {
         hotspot = {50, 72}, -- one pixel higher
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {50, 58}
      }
   },

   aihints = {
      prohibited_till = 400,
      basic_amount = 1,
      weak_ai_limit = 2,
      very_weak_ai_limit = 1,
   },

   working_positions = {
      frisians_blacksmith = 1
   },

   inputs = {
      { name = "iron", amount = 7 },
      { name = "log", amount = 7 },
      { name = "reed", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- "return=skipped" causes 10 sec delay
            -- time total: 12 * 70.167 + 10 = 852 sec
            "call=produce_shovel",
            "call=produce_pick",
            "call=produce_hammer",
            "call=produce_fire_tongs",
            "call=produce_felling_ax",
            "call=produce_scythe",
            "call=produce_needles",
            "call=produce_basket",
            "call=produce_kitchen_tools",
            "call=produce_bread_paddle",
            "call=produce_fishing_net",
            "call=produce_hunting_spear",
            "return=skipped"
         },
      },
      produce_basket = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a basket because ...
         descname = _("making a basket"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs basket",
            "consume=reed log",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=basket"
         },
      },
      produce_bread_paddle = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _("making a bread paddle"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs bread_paddle",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=bread_paddle"
         },
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a felling ax because ...
         descname = _("forging a felling ax"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs felling_ax",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=felling_ax"
         },
      },
      produce_fire_tongs = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _("making fire tongs"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs fire_tongs",
            "consume=iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=fire_tongs"
         },
      },
      produce_fishing_net = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _("making a fishing net"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs fishing_net",
            "consume=reed:2",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=fishing_net"
         },
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _("making a hammer"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs hammer",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=hammer"
         },
      },
      produce_hunting_spear = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _("making a hunting spear"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs hunting_spear",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=hunting_spear"
         },
      },
      produce_kitchen_tools = {
         -- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
         descname = _("making kitchen tools"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs kitchen_tools",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=kitchen_tools"
         },
      },
      produce_needles = {
         -- TRANSLATORS: Completed/Skipped/Did not start making needles because ...
         descname = _("making needles"),
         actions = {
            -- time: 27.967 + 35 + 2 * 3.6 = 70.167 sec
            "return=skipped unless economy needs needles",
            "consume=iron",
            "sleep=duration:27s967ms",
            "animate=working duration:35s",
            "produce=needles:2"
         },
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a pick because ...
         descname = _("forging a pick"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs pick",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=pick"
         },
      },
      produce_scythe = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _("making a scythe"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs scythe",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=scythe"
         },
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _("making a shovel"),
         actions = {
            -- time: 31.567 + 35 + 3.6 = 70.167 sec
            "return=skipped unless economy needs shovel",
            "consume=log iron",
            "sleep=duration:31s567ms",
            "animate=working duration:35s",
            "produce=shovel"
         },
      },
   },
}

pop_textdomain()
