push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_liana_cutters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Liana Cutter's Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 5,
   },
   return_on_dismantle = {
      log = 3
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 45}},
      unoccupied = {hotspot = {39, 45}}
   },

   aihints = {
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 2,
      collects_ware_from_map = "liana",
      requires_supporters = true,
   },

   working_positions = {
      amazons_liana_cutter = 1
   },


   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start making clay because ...
         descname = _"cutting lianas",
         actions = {
            "return=skipped unless economy needs liana",
            "callworker=cut",
            "sleep=duration:30s",
         },
      },
   },
   out_of_resource_notification = {
      -- TRANSLATORS: Short for "No Trees to cut lianas" for clay pits
      title = _"No Trees",
      heading = _"No Tree to Cut Lianas",
      message = pgettext ("amazons_building", "The liana Cutter working at this site can’t find any tree in his work area. Consider dismantling this liana cutter's hut and rebuilding it somewhere else."),
      productivity_threshold = 33
   },
}

pop_textdomain()
