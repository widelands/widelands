push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 2,
      granite = 2,
      marble = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      planks = 1,
      granite = 1,
      marble = 1,
      marble_column = 1
   },

   animation_directory = dirname,
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 89, 82 },
      },
   },

   aihints = {
      basic_amount = 1,
      space_consumer = true,
      prohibited_till = 510,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      empire_farmer = 1
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
         -- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
         descname = _("planting wheat"),
         actions = {
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _("harvesting wheat"),
         actions = {
            "callworker=harvest",
            "sleep=duration:4s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("empire_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
