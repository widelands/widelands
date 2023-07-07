push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_lumberjacks_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Lumberjack’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3
   },
   return_on_dismantle = {
      log = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 43, 45 },
      },
      unoccupied = {
         hotspot = { 43, 45 },
      },
   },

   spritesheets = {
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 42, 44 }
      },
   },

   aihints = {
      basic_amount = 1,
   },

   working_positions = {
      barbarians_lumberjack = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
         descname = _("felling trees"),
         actions = {
            "callworker=harvest",
            "sleep=duration:20s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Trees"),
      heading = _("Out of Trees"),
      message = pgettext("barbarians_building", "The lumberjack working at this lumberjack’s hut can’t find any trees in his work area. You should consider dismantling or destroying the building or building a ranger’s hut."),
      productivity_threshold = 60
   },
}

pop_textdomain()
