push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_foresters_house_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Forester’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   enhancement = {
        name = "europeans_foresters_house_advanced",
        enhancement_cost = {
          marble_column = 1,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 1,
          quartz = 1,
          diamond = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 37, 44 },
      }
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 7200
   },

   working_positions = {
      europeans_forester_normal = 1
   },

   inputs = {
      { name = "water", amount = 4 },
   },
   
   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "consume=water",
            "callworker=plant",
            "sleep=duration:15s"
         }
      },
   },
}

pop_textdomain()
