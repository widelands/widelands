push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_beekeepers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Beekeepers House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      planks = 2,
      reed = 2,
      granite = 1
   },
   return_on_dismantle = {
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 33, 64 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 33, 64 },
      }
   },

   aihints = {
      collects_ware_from_map = "honey",
      requires_supporters = true
   },

   working_positions = {
      europeans_beekeeper = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start spinning gold because ...
         descname = _"making honey",
         actions = {
            "return=skipped unless economy needs honey",
            "callworker=bees",
            "sleep=duration:45s"
         }
      },
   },
   
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Flowers",
      heading = _"Out of Flowers",
      message = pgettext ("europeans_building", "The beekeeper working at this beekeepers’s house can’t find any flowering fields or bushes in his work area. You should consider building another farm or berry farm nearby, or dismantling or destroying this building."),
      productivity_threshold = 8
   },
}

pop_textdomain()
