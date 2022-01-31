push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "empire_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Charcoal Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      marble = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      marble = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 51, 48 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 51, 48 },
      },
   },

   aihints = {
      prohibited_till = 690
   },

   working_positions = {
      empire_charcoal_burner = 1
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
