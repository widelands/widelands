dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_cattlefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Cattle Farm"),
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 80 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
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
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
         descname = pgettext("barbarians_building", "rearing cattle"),
         actions = {
            "return=skipped unless economy needs barbarians_ox",
            "consume=wheat water",
            "sleep=15000",
            "playsound=sound/farm/ox 192",
            "animate=working duration:15s", -- Animation of feeding the cattle
            "recruit=barbarians_ox"
         }
      },
   },
}
