push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_hunters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Hunter’s House"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      planks = 1,
      granite = 1
   },
   return_on_dismantle = {
      granite = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 36, 44 },
      },
   },

   aihints = {
      prohibited_till = 380
   },

   working_positions = {
      atlanteans_hunter = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
         descname = _("hunting"),
         actions = {
            "callworker=hunt",
            "sleep=duration:35s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of Game" for a resource
      title = _("No Game"),
      -- TRANSLATORS: "Game" means animals that you can hunt
      heading = _("Out of Game"),
      -- TRANSLATORS: "game" means animals that you can hunt
      message = pgettext("atlanteans_building", "The hunter working out of this hunter’s house can’t find any game in his work area."),
      productivity_threshold = 0
   },
}

pop_textdomain()
