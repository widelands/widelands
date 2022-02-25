push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Mill"),
   icon = dirname .. "menu.png",
   size = "medium",

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
         fps = 15,
         frames = 19,
         rows = 5,
         columns = 4,
         hotspot = { 41, 87 }
      },
   },

   aihints = {
      prohibited_till = 540
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
         descname = _("grinding wheat"),
         actions = {
            "return=skipped unless economy needs flour",
            "consume=wheat",
            "sleep=duration:5s",
            "playsound=sound/mill/mill_turning priority:90% allow_multiple",
            "animate=working duration:10s",
            "produce=flour"
         }
      },
   },
}

pop_textdomain()
