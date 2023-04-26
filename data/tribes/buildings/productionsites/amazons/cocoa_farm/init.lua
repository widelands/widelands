push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_cocoa_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Cocoa Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      rubber = 2,
      granite = 2,
   },
   return_on_dismantle = {
      log = 2,
      rubber = 1,
      granite = 1,
   },

   animation_directory = dirname,
   animations = { unoccupied = {hotspot = {92, 73}}},
   spritesheets = { idle = {
      hotspot = {92, 112},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }},

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
      very_weak_ai_limit = 2,
      weak_ai_limit = 3,
   },

   working_positions = {
      amazons_cocoa_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=plant_cocoa",
            "call=harvest_cocoa",
         }
      },
      plant_cocoa = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting cocoa"),
         actions = {
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cocoa = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting cocoa"),
         actions = {
            "callworker=harvest",
            "sleep=duration:4s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("amazons_building", "The cocoa farmer working at this cocoa farm has no cleared soil to plant her seeds."),
      productivity_threshold = 20
   },
}

pop_textdomain()
