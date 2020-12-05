push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_foresters_house_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Forester’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 54 },
      },
   },

   aihints = {
      space_consumer = true
   },

   working_positions = {
      europeans_forester_advanced = 1
   },

   inputs = {
      { name = "water", amount = 10 },
   },
   
   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start enhancing the fertility of land because ...
         descname = _"working",
         actions = {
            "call=plant",
            "call=plant",
            "callworker=check",
            "call=terraform",
            "call=terraform",
            "call=terraform",
            "call=terraform",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
         descname = _"planting trees",
         actions = {
            "consume=water",
            "callworker=plant",
            "sleep=duration:12s"
         }
      },
      terraform = {
         -- TRANSLATORS: Completed/Skipped/Did not start enhancing the fertility of land because ...
         descname = _"enhancing the fertility of land",
         actions = {
            "consume=water:2",
            "callworker=terraform",
            "sleep=duration:10s"
         }
      },
   },
}

pop_textdomain()
