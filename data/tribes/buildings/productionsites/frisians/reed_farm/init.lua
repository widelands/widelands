push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_reed_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Reed Farm"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 2,
      --granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {40, 72},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      build = {
         hotspot = {40, 54},
         frames = 3,
         columns = 3,
         rows = 1,
      }
   },
   animations = {
      unoccupied = {
         hotspot = {40, 54}
      }
   },

   aihints = {
      space_consumer = true,
      very_weak_ai_limit = 2,
      weak_ai_limit = 3,
      basic_amount = 2,
   },

   working_positions = {
      frisians_reed_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- nominal transition from tiny to ripe field (the longest possible transition is double):
            --   22 + 28 + 40 = 90 sec
            --   see data/tribes/immovables/reedfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 1):
            --   2 + (90 - (2.333 * 1.8 + 18)) / 54.8 = 3.24
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total (avg. for 3 nearest fields): 29.4 + 25.4 = 54.8 sec
            -- min. time total:                             28.2 + 24.2 = 52.4 sec
            -- max. time total:                             35.4 + 31.4 = 66.8 sec
            "call=plant_reed",
            "call=harvest_reed",
         }
      },
      plant_reed = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting reed"),
         actions = {
            -- time of worker: 10.2-17.4 sec
            -- min. time: 10.2 + 18 = 28.2 sec
            -- max. time: 17.4 + 18 = 35.4 sec
            "callworker=plantreed",
            "sleep=duration:18s"
         }
      },
      harvest_reed = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting reed"),
         actions = {
            -- time of worker: 19.2-26.4 sec
            -- min. time: 19.2 + 5 = 24.2 sec
            -- max. time: 26.4 + 5 = 31.4 sec
            "callworker=harvestreed",
            "sleep=duration:5s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("frisians_building", "The reed farmer working at this reed farm has no cleared soil to plant his seeds."),
      productivity_threshold = 20
   },
}

pop_textdomain()
