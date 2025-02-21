push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_piggery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Piggery"),
   icon = dirname .. "menu.png",
   size = "big",

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
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 89, 82 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 89, 82 },
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
            -- time: 14.8 + 30 + 2 * 3.6 = 52 sec
            "return=skipped unless economy needs meat",
            "consume=water wheat",
            "sleep=duration:14s800ms",
            "playsound=sound/farm/farm_animal priority:40% allow_multiple",
            "animate=working duration:30s",
            "produce=meat:2"
         }
      },
   },
}

pop_textdomain()
