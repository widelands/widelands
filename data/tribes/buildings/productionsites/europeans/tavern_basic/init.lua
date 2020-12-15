push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_tavern_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Tavern"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_tavern_level_1",
        enhancement_cost = {
          planks = 1,
          brick = 1,
          grout = 1,
        },
        enhancement_return_on_dismantle = {
          granite = 1
        },
   },

   buildcost = {
      planks = 3,
      reed = 3,
      granite = 1
   },
   return_on_dismantle = {
      log = 3,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 58 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 41, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 41, 58 },
      },
   },

   aihints = {
      prohibited_till = 3600
   },

   working_positions = {
      europeans_baker_basic = 1,
      europeans_smoker_basic = 1
   },

   inputs = {
      { name = "water", amount = 6 },
      { name = "cornmeal", amount = 6 },
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ration_basic",
            "call=produce_ration"
         }
      },
      produce_ration_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "sleep=duration:45s",
            "consume=water cornmeal fish,meat",
            "animate=working duration:45s",
            "produce=ration"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "return=skipped unless economy needs ration",
            "return=skipped when economy needs water",
            "sleep=duration:60s",
            "consume=water:2 cornmeal:2 fish,meat:2",
            "animate=working duration:60s",
            "produce=ration:2"
         }
      },
   },
}

pop_textdomain()
