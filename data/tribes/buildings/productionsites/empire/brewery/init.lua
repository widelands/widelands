push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 1,
      planks = 2,
      granite = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 39, 62 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 39, 62 },
      },
   },

   aihints = {
      prohibited_till = 790,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_brewer = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "wheat", amount = 7 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _"brewing beer",
         actions = {
            "return=skipped unless economy needs beer",
            "consume=water wheat",
            "sleep=duration:30s",
            "playsound=sound/empire/beerbubble priority:40% allow_multiple",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}

pop_textdomain()
