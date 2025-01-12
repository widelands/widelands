push_textdomain("tribes")

local dirname = path.dirname(__file__)

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
            -- longest possible transition from tiny to ripe field:
            --   2 * (22 + 28 + 40) = 180 sec
            --   see data/tribes/immovables/grapevine/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for reliable 100% productivity: 5
            --   with 4 fields, this inequation is not fulfilled:
            --   (180 + 48 - (5 + 2.5 * 1.8 + 5)) / 48 < 4
            -- min. time total (free 4 nearest fields): 24    + 24    = 48    sec
            -- time total (free all 5 fields):          25.08 + 25.08 = 50.16 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting grapevines because ...
         descname = _("planting grapevines"),
         actions = {
            -- time of worker: 20.08 sec, min. time for 4 fields 19 sec
            -- min. time (4 fields): 19    + 5 = 24    sec
            -- time:                 20.08 + 5 = 25.08 sec
            "callworker=plant",
            "sleep=duration:5s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting grapevines because ...
         descname = _("harvesting grapes"),
         actions = {
            -- time of worker: 20.08 sec, min. time for 4 fields 19 sec
            -- min. time (4 fields): 19    + 5 = 24    sec
            -- time:                 20.08 + 5 = 25.08 sec
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
