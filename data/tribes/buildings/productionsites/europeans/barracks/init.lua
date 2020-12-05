push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Barracks"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 3,
      spidercloth = 3,
      brick = 3,
      grout = 3,
      quartz = 2,
      diamond = 1
   },
   return_on_dismantle = {
      log = 2,
      spidercloth = 1,
      granite = 2,
      quartz = 2,
      diamond = 1
   },

   animations = {
      idle = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 38 },
      },
      build = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 50, 38 },
      },
      working = { -- TODO(GunChleoc): make animations
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 50, 38 },
      }
   },

   aihints = {
      prohibited_till = 3600,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
     --normal_ai_limit = 4
   },

   working_positions = {
      europeans_trainer = 1
   },

   inputs = {
      { name = "europeans_recruit", amount = 8 },
      { name = "spear_wooden", amount = 8 },
      { name = "armor", amount = 4 },
      { name = "tabard", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("atlanteans_building", "recruiting soldier"),
         actions = {
            "return=skipped unless economy needs europeans_soldier",
            "sleep=duration:30s",
            "return=skipped when not site has spear_wooden",
            "return=skipped when not site has armor and not site has tabard",
            "consume=armor,tabard spear_wooden europeans_recruit",
            "animate=working duration:30s",
            "recruit=europeans_soldier"
         }
      },
   }
}

pop_textdomain()
