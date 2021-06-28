dirname = "tribes/buildings/productionsites/empire/mill/"

push_textdomain("scenario_emp04.wmf")

descriptions:new_productionsite_type {
   name = "empire_mill1",
   descname = pgettext("empire_building", "Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "empire_mill",
      enhancement_cost = {
         log = 1,
         granite = 1,
         marble = 1
      },
      enhancement_return_on_dismantle = {}
   },

   buildcost = {
      log = 3,
      granite = 3,
      marble = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      marble = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 41, 87 },
      },
   },

   spritesheets = {
      working = {
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 41, 87 }
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
         descname = pgettext("empire_building", "grinding wheat"),
         actions = {
            "sleep=duration:5s",
            "return=skipped unless economy needs flour",
            "consume=wheat:2",
            "playsound=sound/mill/mill_turning priority:90% allow_multiple",
            "animate=working duration:10s",
            "produce=flour"
         }
      },
   },
}
pop_textdomain()
