push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_piggery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Piggery"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      marble = 1
   },


   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 82, 74 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 82, 74 },
      },
   },

   aihints = {
      prohibited_till = 590
   },

   working_positions = {
      empire_pigbreeder = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "wheat", amount = 7 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start raising pigs because ...
         descname = _("raising pigs"),
         actions = {
            "return=skipped unless economy needs meat",
            "consume=water wheat",
            "sleep=duration:10s",
            "playsound=sound/farm/farm_animal priority:40% allow_multiple",
            "animate=working duration:20s",
            "produce=meat"
         }
      },
   },
}

pop_textdomain()
