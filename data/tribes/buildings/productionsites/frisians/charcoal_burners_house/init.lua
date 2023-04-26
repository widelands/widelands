push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_charcoal_burners_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Charcoal Burner’s House"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 2,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {43, 74},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
   },

   animations = {
      unoccupied = {
         hotspot = {43, 56}
      }
   },

   aihints = {
      prohibited_till = 760,
      requires_supporters = true,
   },

   working_positions = {
      frisians_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=erect_stack",
            "call=collect_coal",
         },
      },
      erect_stack = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a charcoal stack because ...
         descname = _("making a charcoal stack"),
         actions = {
            "return=skipped unless economy needs coal",
            "callworker=find_pond",
            "consume=log:3",
            "callworker=make_stack",
            "sleep=duration:15s",
         },
      },
      collect_coal = {
         -- TRANSLATORS: Completed/Skipped/Did not start collecting coal because ...
         descname = _("collecting coal"),
         actions = {
            "sleep=duration:15s",
            "callworker=collect_coal",
         },
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=log:3",
            "produce=coal",
         },
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Ponds"),
      heading = _("Out of Clay Ponds"),
      message = pgettext ("frisians_building", "The charcoal burner working at this charcoal burner’s house can’t find any clay ponds in his work area. Please make sure there is a working clay pit nearby and the charcoal burner’s house is supplied with all needed wares, or consider dismantling or destroying this building."),
      productivity_threshold = 12
   },
}

pop_textdomain()
