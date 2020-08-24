dirname = "tribes/buildings/productionsites/empire/mill/"

push_textdomain("scenario_emp04.wmf")

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_mill1",
   descname = pgettext("empire_building", "Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "empire_mill",
      buildcost = {
         log = 1,
         granite = 1,
         marble = 1
      },
      return_on_dismantle = {
      }
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
         descname = "grinding wheat",
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
