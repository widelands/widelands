push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Charcoal Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 3,
      planks = 1
   },
   return_on_dismantle = {
      log = 2,
      granite = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 47, 57 },
      },
   },

   spritesheets = {
      working = {
         frames = 5,
         rows = 3,
         columns = 2,
         hotspot = { 47, 60 }
      },
   },

   aihints = {
      prohibited_till = 690
   },

   working_positions = {
      atlanteans_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _("producing coal"),
         actions = {
            "return=skipped unless economy needs coal",
            "consume=log:6",
            "sleep=duration:30s",
            "animate=working duration:1m30s", -- Charcoal fires will burn for some days in real life
            "produce=coal"
         }
      },

   },
}

pop_textdomain()
