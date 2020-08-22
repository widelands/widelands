dirname = "tribes/buildings/productionsites/empire/mill/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_mill2",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
      log = 1,
      granite = 1,
      marble = 1
   },
   return_on_dismantle_on_enhanced = {
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 87 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 41, 87 },
         fps = 25
      },
   },

   aihints = {
   },

   working_positions = {
      empire_miller = 1
   },

   inputs = {
      { name = "wheat", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding wheat because ...
         descname = _"grinding wheat",
         actions = {
            "sleep=5000",
            "return=skipped unless economy needs flour",
            "consume=wheat",
            "playsound=sound/mill/mill_turning 240",
            "animate=working duration:10s",
            "produce=flour"
         }
      },
   },
}
pop_textdomain()
