push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "amazons_cassava_plantation",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Cassava Plantation"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      rope = 2,
      granite = 2,
   },
   return_on_dismantle = {
      log = 2,
      rope = 1,
      granite = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {92, 73}},
      unoccupied = {hotspot = {92, 73}},
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
   },

   working_positions = {
      amazons_cassava_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=plant_cassava",
            "call=harvest_cassava",
         }
      },
      plant_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting cassava because ...
         descname = _("planting cassava"),
         actions = {
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting cassava because ...
         descname = _("harvesting cassava"),
         actions = {
            "callworker=harvest",
            "sleep=duration:4s",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext ("amazons_building", "The farmer working at this cassava plantation has no cleared soil to plant her seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
