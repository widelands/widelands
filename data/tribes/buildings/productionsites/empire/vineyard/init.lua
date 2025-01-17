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
            -- the longest possible transition from tiny to ripe field:
            --   2 * (22 + 28 + 40) = 180 sec
            --   see data/tribes/immovables/grapevine/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for 100% productivity in worst case (nearest fields, longest field transition):
            --   1 + (180 - (5 + 2.5 * 1.8 + 5)) / 48 = 4.448
            -- calculation of productivity with 4 fields:
            --   irwinhall(8, scale=1/8).cdf(4/4.448) = 0.999996
            --   using SciPy 1.14+
            -- calculation of productivity with 3 fields:
            --   irwinhall(6, scale=1/6).cdf(3/4.543) = 0.911
            -- min. time total (free 4 nearest fields):  24   + 24   = 48   sec
            -- max. time total (free 4 furthest fields): 25.8 + 25.8 = 51.6 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting grapevines because ...
         descname = _("planting grapevines"),
         actions = {
            -- time of worker: 19-20.8 sec
            -- min. time: 19   + 5 = 24   sec
            -- max. time: 20.8 + 5 = 25.8 sec
            "callworker=plant",
            "sleep=duration:5s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting grapevines because ...
         descname = _("harvesting grapes"),
         actions = {
            -- time of worker: 19-20.8 sec
            -- min. time: 19   + 5 = 24   sec
            -- max. time: 20.8 + 5 = 25.8 sec
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
