push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_stone_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Stone Workshop"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      balsa = 2,
      ironwood = 2,
      rubber = 2,
      granite = 2,
   },
   return_on_dismantle = {
      balsa = 1,
      ironwood = 1,
      rubber = 1,
      granite = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {43, 44}},
      unoccupied = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 44},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3,
   },

   working_positions = {
      amazons_stone_carver = 1
   },

   inputs = {
      { name = "granite", amount = 4 },
      { name = "quartz", amount = 7 },
      { name = "log", amount = 6 },
      { name = "ironwood", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time total: 19 * 69 = 1311 sec
            "call=produce_shovel",
            "call=produce_spear_wooden",
            -- firestones as first consumer of quartz to help poor hamlet
            "call=produce_firestones",
            "call=produce_spear_stone_tipped",
            "call=produce_felling_ax",
            "call=produce_spear_hardened",
            "call=produce_pick",
            "call=produce_spear_wooden",
            "call=produce_machete",
            "call=produce_spear_stone_tipped",
            "call=produce_kitchen_tools",
            "call=produce_spear_hardened",
            "call=produce_hammer",
            "call=produce_spear_wooden",
            "call=produce_chisel",
            "call=produce_spear_stone_tipped",
            "call=produce_needles",
            "call=produce_spear_hardened",
            "call=produce_stone_bowl",
         },
      },
      produce_shovel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _("making a shovel"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs shovel",
            "consume=log ironwood",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=shovel"
         },
      },
      produce_felling_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a felling ax because ...
         descname = _("making a felling ax"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs felling_ax",
            "consume=log quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=felling_ax"
         },
      },
      produce_pick = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a pick because ...
         descname = _("making a pick"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs pick",
            "consume=log quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=pick"
         },
      },
      produce_machete = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a machete because ...
         descname = _("making a machete"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs machete",
            "consume=log quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=machete"
         },
      },
      produce_kitchen_tools = {
         -- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
         descname = _("making kitchen tools"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs kitchen_tools",
            "consume=log granite",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=kitchen_tools"
         },
      },
      produce_hammer = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _("making a hammer"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs hammer",
            "consume=log granite",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=hammer"
         },
      },
      produce_chisel = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a chisel because ...
         descname = _("making a chisel"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs chisel",
            "consume=quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=chisel"
         },
      },
      produce_needles = {
         -- TRANSLATORS: Completed/Skipped/Did not start making needles because ...
         descname = _("making needles"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs needles",
            "consume=quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=needles"
         },
      },
      produce_stone_bowl = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a stone bowl because ...
         descname = _("making a stone bowl"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs stone_bowl",
            "consume=granite",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=stone_bowl"
         },
      },
      produce_firestones = {
         -- TRANSLATORS: Completed/Skipped/Did not start making firestones because ...
         descname = _("making firestones"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs firestones",
            "consume=quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=firestones"
         },
      },
      produce_spear_wooden = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a wooden spear because ...
         descname = _("making a wooden spear"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs spear_wooden",
            "consume=ironwood",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=spear_wooden"
         },
      },
      produce_spear_stone_tipped = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a stone-tipped spear because ...
         descname = _("making a stone-tipped spear"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs spear_stone_tipped",
            "consume=ironwood quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=spear_stone_tipped"
         },
      },
      produce_spear_hardened = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hardened spear because ...
         descname = _("making a hardened spear"),
         actions = {
            -- time: 30.4 + 35 + 3.6 = 69 sec
            "return=skipped unless economy needs spear_hardened",
            "consume=ironwood log quartz",
            "sleep=duration:30s400ms",
            "animate=working duration:35s",
            "produce=spear_hardened"
         },
      },
   },
}

pop_textdomain()
