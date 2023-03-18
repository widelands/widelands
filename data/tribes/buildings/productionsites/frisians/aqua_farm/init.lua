push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_aqua_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Aqua Farm"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 2,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {40, 71},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {40, 53}
      }
   },

   aihints = {
      prohibited_till = 760,
      requires_supporters = true
   },

   working_positions = {
      frisians_fisher = 1
   },

   inputs = {
      { name = "water", amount = 5 },
      { name = "fruit", amount = 2 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=breed_fish",
            "call=fish_pond",
         },
      },
      breed_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _("breeding fish"),
         actions = {
            "return=skipped unless economy needs fish",
            "return=failed unless site has water:2",
            "return=failed unless site has fruit",
            "callworker=breed_in_pond",
            "consume=fruit water:2",
            "sleep=duration:23s",
         },
      },
      fish_pond = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _("fishing"),
         actions = {
            "return=skipped unless economy needs fish",
            "sleep=duration:9s",
            "callworker=fish_in_pond",
         },
      },
      encyclopedia = {
         -- just a dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=fruit water:2",
            "produce=fish",
         },
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Ponds"),
      heading = _("Out of Fish Ponds"),
      message = pgettext ("frisians_building", "The fisher working at this aqua farm can’t find any fish ponds in his work area. Please make sure there is a working clay pit nearby and the aqua farm is supplied with all needed wares, or consider dismantling or destroying this building."),
      productivity_threshold = 12
   },
}

pop_textdomain()
