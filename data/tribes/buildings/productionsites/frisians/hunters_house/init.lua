push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_hunters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Hunter’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {40, 69},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {40, 52}
      }
   },

   aihints = {
      prohibited_till = 480
   },

   working_positions = {
      frisians_hunter = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _("hunting"),
         actions = {
            "callworker=hunt",
            "sleep=duration:35s",
            "callworker=hunt",
            "sleep=duration:35s",
            "callworker=hunt",
            "sleep=duration:35s",
            "callworker=hunt",
            "sleep=duration:35s",
            "callworker=hunt",
            "sleep=duration:35s",
            "produce=fur"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of Game" for a resource
      title = _("No Game"),
      -- TRANSLATORS: "Game" means animals that you can hunt
      heading = _("Out of Game"),
      -- TRANSLATORS: "game" means animals that you can hunt
      message = pgettext("frisians_building", "The hunter working out of this hunter’s house can’t find any game in his work area."),
      productivity_threshold = 33
   },
}

pop_textdomain()
