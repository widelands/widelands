push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_cocoa_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Cocoa Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      rubber = 2,
      granite = 2,
   },
   return_on_dismantle = {
      log = 2,
      rubber = 1,
      granite = 1,
   },

   animation_directory = dirname,
   animations = { unoccupied = {hotspot = {92, 73}}},
   spritesheets = {
      build = {
         hotspot = {92, 73},
         frames = 5,
         columns = 3,
         rows = 2
      },
      idle = {
         hotspot = {92, 112},
         fps = 8,
         frames = 4,
         columns = 2,
         rows = 2
      }
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
      very_weak_ai_limit = 2,
      weak_ai_limit = 3,
   },

   working_positions = {
      amazons_cocoa_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- nominal transition from sapling to old tree (the longest possible transition is double):
            --   80 + 60 + 60 = 200 sec
            --   in each tree stage, it may disappear with certain small probability
            --   see data/tribes/immovables/trees/cocoa/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 2):
            --   3 + (200 - (6 + 2.333 * 1.8 + 10)) / 56.8 = 6.17
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total (avg. for 6 nearest fields): 30.4 + 26.4 = 56.8 sec
            -- min. time total:                             29.2 + 25.2 = 54.4 sec
            -- max. time total:                             47.2 + 43.2 = 90.4 sec
            "call=plant_cocoa",
            "call=harvest_cocoa",
         }
      },
      plant_cocoa = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting cocoa"),
         actions = {
            -- time of worker: 19.2-37.2 sec
            -- min. time: 19.2 + 10 = 29.2 sec
            -- max. time: 37.2 + 10 = 47.2 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cocoa = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting cocoa"),
         actions = {
            -- time of worker: 21.2-39.2 sec
            -- min. time: 21.2 + 4 = 25.2 sec
            -- max. time: 39.2 + 4 = 43.2 sec
            "callworker=harvest",
            "sleep=duration:4s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("amazons_building", "The cocoa farmer working at this cocoa farm has no cleared soil to plant her seeds."),
      productivity_threshold = 20
   },
}

pop_textdomain()
