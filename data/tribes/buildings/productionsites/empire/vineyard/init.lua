push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_vineyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Vineyard"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      planks = 2,
      granite = 1,
      marble = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      marble = 2
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 53, 53 },
      },
   },

   aihints = {
      space_consumer = true,
      basic_amount = 2,
      prohibited_till = 490,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3

   },

   working_positions = {
      empire_vinefarmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting grapevines because ...
         descname = _("planting grapevines"),
         actions = {
            "callworker=plant",
            "sleep=duration:5s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting grapevines because ...
         descname = _("harvesting grapes"),
         actions = {
            "callworker=harvest",
            "sleep=duration:5s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("empire_building", "The vine farmer working at this vineyard has no cleared soil to plant his grapevines."),
      productivity_threshold = 30
   },
}

pop_textdomain()
