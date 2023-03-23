push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_collectors_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Fruit Collector’s House"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 2,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {40, 72},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {40, 55}
      }
   },

   aihints = {
      prohibited_till = 470,
      requires_supporters = true
   },

   working_positions = {
      frisians_fruit_collector = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start gathering berries because ...
         descname = _("gathering berries"),
         actions = {
            "sleep=duration:21s",
            "callworker=harvest",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Bushes"),
      heading = _("Out of Berries"),
      message = pgettext ("frisians_building", "The fruit collector working at this fruit collector’s house can’t find any berry bushes in his work area. You should consider building another berry farm, or dismantling or destroying this building."),
      productivity_threshold = 8
   },
}

pop_textdomain()
