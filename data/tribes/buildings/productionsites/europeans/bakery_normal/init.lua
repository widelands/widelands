push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_bakery_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_bakery_advanced",
        enhancement_cost = {
          marble_column = 2,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 2,
          quartz = 1,
          diamond = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 63 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 52, 75 },
         fps = 20
      }
   },

   aihints = {
   },

   working_positions = {
      europeans_baker_normal = 1
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
            "consume=water:4 cornmeal:2 flour:2",
            "sleep=duration:20s",
            "animate=working duration:25s",
            "produce=europeans_bread:2",
            "animate=working duration:25s",
            "produce=europeans_bread"
         }
      },
   },
}

pop_textdomain()
