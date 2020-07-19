dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_spiderfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Spider Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 2
   },
   return_on_dismantle = {
      granite = 1,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 87, 75 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 87, 75 },
      }
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 350
   },

   working_positions = {
      atlanteans_spiderbreeder = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "corn", amount = 7 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "return=skipped unless economy needs spider_silk",
            "consume=corn water",
            "sleep=25000",
            "animate=working 30000",
            "produce=spider_silk"
         }
      },
   },
}
