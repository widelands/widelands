dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_beekeepers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Bee-keeper´s House"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 46, 47 },
      }
   },

   aihints = {
      prohibited_till = 600,
   },

   working_positions = {
      frisians_beekeeper = 1
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=45000",
            "worker=bees"
         }
      },
   },
}
