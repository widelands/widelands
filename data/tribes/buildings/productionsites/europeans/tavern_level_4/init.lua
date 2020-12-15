push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_tavern_level_4",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Tavern Level 4"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_tavern_level_5",
        enhancement_cost = {
          planks = 1,
          brick = 1,
          grout = 1,
          spidercloth = 1,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          log = 1,
          granite = 1,
          quartz = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 53, 68 },
         fps = 20
      }
   },

   aihints = {
      prohibited_till = 5400
   },

   working_positions = {
      europeans_baker_normal = 1,
      europeans_smoker_normal = 1
   },

   inputs = {
      { name = "water", amount = 8 },
      { name = "cornmeal", amount = 8 },
      { name = "flour", amount = 8 },
      { name = "fish", amount = 6 },
      { name = "meat", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration_basic",
            "call=produce_ration",
            "call=produce_snack_basic",
            "call=produce_snack"
         }
      },
      produce_ration_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "sleep=duration:30s",
            "consume=water cornmeal fish,meat",
            "animate=working duration:30s",
            "produce=ration"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "return=skipped unless economy needs ration",
            "return=skipped when economy needs water",
            "sleep=duration:40s",
            "consume=water:2 cornmeal:2 fish,meat:2",
            "animate=working duration:40s",
            "produce=ration:2"
         }
      },
      produce_snack_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "sleep=duration:40s",
            "consume=water:2 cornmeal flour fish,meat:2",
            "animate=working duration:40s",
            "produce=snack"
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _"preparing a snack",
         actions = {
            "return=skipped unless economy needs snack",
            "return=skipped when economy needs water",
            "sleep=duration:55s",
            "consume=water:4 cornmeal:3 flour fish,meat:4",
            "animate=working duration:55s",
            "produce=snack:3"
         }
      },
   },
}

pop_textdomain()
