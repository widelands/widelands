push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_smelting_works",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Smelting Works"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      granite = 4,
      planks = 1,
      spidercloth = 1
   },
   return_on_dismantle = {
      granite = 3
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 57, 72 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 57, 72 },
      }
   },

   aihints = {
      prohibited_till = 700,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_smelter = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron_ore", amount = 8 },
      { name = "gold_ore", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=smelt_iron",
            "call=smelt_gold",
            "call=smelt_iron_2",
         }
      },
      -- 2 identical programs for iron to prevent unnecessary skipping penalty
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _("smelting iron"),
         actions = {
            "return=skipped unless economy needs iron",
            "consume=iron_ore coal",
            "sleep=duration:25s",
            "playsound=sound/metal/fizzle priority:15% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron"
         }
      },
      smelt_iron_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
         descname = _("smelting iron"),
         actions = {
            "return=skipped unless economy needs iron",
            "consume=iron_ore coal",
            "sleep=duration:25s",
            "playsound=sound/metal/fizzle priority:15% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/ironping priority:60%",
            "produce=iron"
         }
      },
      smelt_gold = {
         -- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
         descname = _("smelting gold"),
         actions = {
            "return=skipped unless economy needs gold",
            "consume=gold_ore coal",
            "sleep=duration:25s",
            "playsound=sound/metal/fizzle priority:15% allow_multiple",
            "animate=working duration:35s",
            "playsound=sound/metal/goldping priority:60%",
            "produce=gold"
         }
      },
   },
}

pop_textdomain()
