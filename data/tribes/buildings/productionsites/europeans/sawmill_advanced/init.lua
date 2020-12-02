push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_sawmill_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Sawmill"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 54, 70 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 54, 70 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_carpenter_advanced = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
         descname = _"sawing logs",
         actions = {
            "return=skipped unless economy needs planks",
            "return=skipped when economy needs log",
            "consume=log:4",
            "return=skipped when not site has log:4",
            "sleep=duration:12s", -- Much faster than barbarians' wood hardener
            "playsound=sound/sawmill/sawmill priority:40% allow_multiple",
            "animate=working duration:20s", -- Much faster than barbarians' wood hardener
            "produce=planks:3"
         }
      },
   },
}

pop_textdomain()
