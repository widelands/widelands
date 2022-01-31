push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_stonemasons_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Stonemason’s House"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1,
      marble = 3 -- Someone who works on marble should like marble.
   },
   return_on_dismantle = {
      granite = 1,
      marble = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 58, 61 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 58, 61 },
      },
   },

   aihints = {
      basic_amount = 1,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_stonemason = 1
   },

   inputs = {
      { name = "marble", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start sculpting a marble column because ...
         descname = _("sculpting a marble column"),
         actions = {
            "return=skipped unless economy needs marble_column",
            "consume=marble:2",
            "sleep=duration:50s",
            "playsound=sound/stonecutting/stonemason priority:50% allow_multiple",
            "animate=working duration:32s",
            "produce=marble_column"
         }
      },
   },
}

pop_textdomain()
