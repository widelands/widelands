push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_vineyard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Vineyard"),
   icon = dirname .. "menu.png",
   size = "mine",

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
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 59, 65 },
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
            -- nominal transition from tiny to ripe field (the longest possible transition is double):
            --   22 + 28 + 40 = 90 sec
            --   see data/tribes/immovables/grapevine/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 1):
            --   2 + (90 - (5 + 2.5 * 1.8 + 5)) / 48 = 3.57
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total (avg. for 4 nearest fields): 24   + 24   = 48   sec
            -- min. time total:                             22.2 + 22.2 = 44.4 sec
            -- max. time total:                             29.4 + 29.4 = 58.8 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting grapevines because ...
         descname = _("planting grapevines"),
         actions = {
            -- time of worker: 17.2-24.4 sec
            -- min. time: 17.2 + 5 = 22.2 sec
            -- max. time: 24.4 + 5 = 29.4 sec
            "callworker=plant",
            "sleep=duration:5s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting grapevines because ...
         descname = _("harvesting grapes"),
         actions = {
            -- time of worker: 17.2-24.4 sec
            -- min. time: 17.2 + 5 = 22.2 sec
            -- max. time: 24.4 + 5 = 29.4 sec
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
