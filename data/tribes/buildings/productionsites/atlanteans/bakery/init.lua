push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      planks = 2,
      granite = 3
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 54, 74 },
      },
   },

   spritesheets = {
      working = {
         fps = 20,
         frames = 5,
         columns = 5,
         rows = 1,
         hotspot = { 54, 74 },
      },
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 590
   },

   working_positions = {
      atlanteans_baker = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "cornmeal", amount = 4 },
      { name = "blackroot_flour", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = pgettext("atlanteans_building", "baking bread"),
         actions = {
            "return=skipped unless economy needs atlanteans_bread",
            "consume=water:2 blackroot_flour cornmeal",
            "animate=working duration:35s",
            "sleep=duration:30s",
            "produce=atlanteans_bread:2"
         }
      },
   },
}

pop_textdomain()
