push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_fishers_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Fisher’s House"),
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
         hotspot = { 42, 60 },
      },
   },

   aihints = {
      needs_water = true,
      prohibited_till = 410
   },

   working_positions = {
      empire_fisher = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _("fishing"),
         actions = {
            "callworker=fish",
            "sleep=duration:10s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fish"),
      heading = _("Out of Fish"),
      message = pgettext("empire_building", "The fisher working out of this fisher’s house can’t find any fish in his work area."),
   },
}

pop_textdomain()
