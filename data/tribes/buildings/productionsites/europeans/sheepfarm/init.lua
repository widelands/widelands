push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_sheepfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Sheep Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 4,
      reed = 3,
      brick = 2,
      grout = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      log = 3,
      granite = 2,
      marble = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 73, 60 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 73, 60 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_breeder_advanced = 1
   },

   inputs = {
      { name = "water", amount = 6 },
      { name = "wheat", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding sheep because ...
         descname = _"breeding sheep",
         actions = {
            "return=skipped unless economy needs wool",
            "consume=water:2 wheat:2",
            "sleep=duration:25s",
            "playsound=sound/farm/sheep priority:50% allow_multiple",
            "animate=working duration:35s",
            "produce=wool:2"
         }
      },
   },
}

pop_textdomain()
