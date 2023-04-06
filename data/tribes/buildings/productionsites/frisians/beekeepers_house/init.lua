push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_beekeepers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Beekeeper’s House"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      reed = 3
   },
   return_on_dismantle = {
      brick = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {40, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {40, 56}
      }
   },

   aihints = {
      prohibited_till = 620,
      requires_supporters = true
   },

   working_positions = {
      frisians_beekeeper = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "callworker=bees",
            "sleep=duration:45s"
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Flowers"),
      heading = _("Out of Flowers"),
      message = pgettext ("frisians_building", "The beekeeper working at this beekeeper’s house can’t find any flowering fields or bushes in his work area. You should consider building another farm or berry farm nearby, or dismantling or destroying this building."),
      productivity_threshold = 8
   },
}

pop_textdomain()
