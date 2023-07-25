push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_cattlefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Cattle Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      blackwood = 1
   },
   return_on_dismantle = {
      granite = 1,
      blackwood = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 57, 80 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 57, 80 },
      },
   },

   aihints = {
      prohibited_till = 610,
   },

   working_positions = {
      barbarians_cattlebreeder = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "wheat", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
         descname = pgettext("barbarians_building", "rearing cattle"),
         actions = {
            "return=skipped unless economy needs barbarians_ox",
            "consume=wheat water",
            "sleep=duration:15s",
            "playsound=sound/farm/ox priority:50% allow_multiple",
            "animate=working duration:15s", -- Animation of feeding the cattle
            "recruit=barbarians_ox"
         }
      },
   },
}

pop_textdomain()
