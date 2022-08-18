push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_furnace",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Furnace"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 2,
      log = 1,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {50, 69},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working_iron = {
         hotspot = {50, 69},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working_gold = {
         hotspot = {50, 69},
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
      prohibited_till = 700,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      frisians_smelter = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron_ore", amount = 8 },
      { name = "gold_ore", amount = 8 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=smelt_iron",
            "call=smelt_gold",
            "call=smelt_iron_2",
         },
      },
      -- 2 identical programs for iron to prevent unnecessary skipping penalty
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _("smelting iron"),
         actions = {
            "return=skipped unless economy needs iron",
            "consume=coal iron_ore",
            "sleep=duration:25s",
            "playsound=sound/metal/furnace priority:50% allow_multiple",
            "animate=working_iron duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron"
         },
      },
      smelt_iron_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _("smelting iron"),
         actions = {
            "return=skipped unless economy needs iron",
            "consume=coal iron_ore",
            "sleep=duration:25s",
            "playsound=sound/metal/furnace priority:50% allow_multiple",
            "animate=working_iron duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron"
         },
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _("smelting gold"),
         actions = {
            "return=skipped unless economy needs gold",
            "consume=coal gold_ore",
            "sleep=duration:27s",
            "playsound=sound/metal/furnace priority:50% allow_multiple",
            "animate=working_gold duration:35s",
            "playsound=sound/metal/goldping priority:60%",
            "produce=gold"
         },
      },
   },
}

pop_textdomain()
