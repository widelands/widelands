push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_micro_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Micro Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "barbarians_brewery",
      enhancement_cost = {
         log = 3,
         granite = 1,
         reed = 1
      },
      enhancement_return_on_dismantle = {
         log = 1,
         granite = 1
      }
   },

   buildcost = {
      log = 3,
      blackwood = 2,
      granite = 3,
      reed = 2
   },
   return_on_dismantle = {
      log = 1,
      blackwood = 1,
      granite = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 42, 50 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 42, 50 },
      },
   },

   aihints = {
      prohibited_till = 530,
      forced_after = 720
   },

   working_positions = {
      barbarians_brewer = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "wheat", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
         descname = _("brewing beer"),
         actions = {
            "return=skipped unless economy needs beer or workers need experience",
            "consume=water wheat",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=beer"
         }
      },
   },
}

pop_textdomain()
