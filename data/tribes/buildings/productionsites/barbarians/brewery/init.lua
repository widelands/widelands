push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "barbarians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Brewery"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 60, 59 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 60, 59 },
      },
   },

   aihints = {},

   working_positions = {
      barbarians_brewer_master = 1,
      barbarians_brewer = 1,
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "wheat", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start brewing strong beer because ...
         descname = _"brewing strong beer",
         actions = {
            "return=skipped unless economy needs beer_strong",
            "consume=water wheat",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=beer_strong"
         }
      },
   },
}

pop_textdomain()
