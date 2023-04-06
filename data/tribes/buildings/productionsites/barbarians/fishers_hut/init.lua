push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_fishers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Fisher’s Hut"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 4
   },
   return_on_dismantle = {
      log = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 39, 52 },
      },
      unoccupied = {
         hotspot = { 39, 52 },
      },
   },

   spritesheets ={
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 39, 52 }
      },
   },

   aihints = {
      needs_water = true,
      prohibited_till = 490,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      barbarians_fisher = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _("fishing"),
         actions = {
            "callworker=fish",
            "sleep=duration:10s500ms"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fish"),
      heading = _("Out of Fish"),
      message = pgettext("barbarians_building", "The fisher working out of this fisher’s hut can’t find any fish in his work area."),
   },
}

pop_textdomain()
