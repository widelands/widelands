push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_bakery_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 42, 65 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 42, 65 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 43, 65 },
         fps = 2
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_baker_advanced = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "cornmeal", amount = 4 },
      { name = "flour", amount = 4 }
   },
   
   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = pgettext("europeans_building", "baking bread"),
         actions = {
            "return=skipped unless economy needs europeans_bread",
            "consume=water:4 cornmeal flour:3",
            "sleep=duration:20s",
            "animate=working duration:20s",
            "produce=europeans_bread:2",
            "animate=working duration:20s",
            "produce=europeans_bread:2"
         }
      },
   },
}

pop_textdomain()
