push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Farm"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 3,
      granite = 1,
      log = 2,
      reed = 3
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {91, 111},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {91, 111},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      build = {
         hotspot = {91, 90},
         frames = 2,
         columns = 2,
         rows = 1
      }
   },
   animations = {
      unoccupied = {
         hotspot = {91, 90}
      }
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
   },

   working_positions = {
      frisians_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- nominal transition from tiny to ripe field (the longest possible transition is double):
            --   150 + 225 + 250 = 625 sec
            --   see data/tribes/immovables/barleyfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 2):
            --   3 + (625 - (6 + 2.556 * 1.8 + 10)) / 102 = 8.93
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total (avg. for 9 nearest fields): 31.2 + 70.8 = 102 sec
            -- min. time total:                             29.2 + 68.8 =  98 sec
            -- max. time total:                             47.2 + 86.8 = 134 sec
            "call=plant_barley",
            "call=harvest_barley",
         }
      },
      plant_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting barley because ...
         descname = _("planting barley"),
         actions = {
            -- time of worker: 19.2-37.2 sec
            -- min. time: 19.2 + 10 = 29.2 sec
            -- max. time: 37.2 + 10 = 47.2 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting barley because ...
         descname = _("harvesting barley"),
         actions = {
            -- time of worker: 21.2-39.2 sec
            -- min. time: 21.2 + 40 + 4 + 3.6 = 68.8 sec
            -- max. time: 39.2 + 40 + 4 + 3.6 = 86.8 sec
            "callworker=harvest",
            "animate=working duration:40s",
            "sleep=duration:4s",
            "produce=barley" --produces 2 barley per field
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext ("frisians_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
