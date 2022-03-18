push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_horsefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Horse Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 81, 62 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 81, 62 },
      }
   },

   aihints = {
      prohibited_till = 610,
   },

   working_positions = {
      atlanteans_horsebreeder = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "corn", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding horses because ...
         descname = pgettext("atlanteans_building", "breeding horses"),
         actions = {
            "return=skipped unless economy needs atlanteans_horse",
            "consume=corn water",
            "sleep=duration:15s",
            "playsound=sound/farm/horse priority:50% allow_multiple",
            "animate=working duration:15s", -- Feeding cute little foals ;)
            "recruit=atlanteans_horse"
         }
      },

   },
}

pop_textdomain()
