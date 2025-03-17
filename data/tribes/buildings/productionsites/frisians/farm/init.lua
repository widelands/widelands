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
            -- longest possible transition from tiny to ripe field:
            --   2 * (150 + 225 + 250) = 1250 sec
            --   see data/tribes/immovables/barleyfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for reliable 100% productivity: 13
            --   with 12 fields, this inequation is not fulfilled:
            --   (1250 + 102.8 - (6 + 2.667 * 1.8 + 10)) / 102.8 < 12
            -- min. time total (free 12 nearest fields):  31.6  + 71.2  = 102.8  sec
            -- min. time total (free 13 nearest fields):  31.97 + 71.57 = 103.54 sec
            -- max. time total (free 13 furthest fields): 40.55 + 80.15 = 120.7  sec
            "call=plant_barley",
            "call=harvest_barley",
         }
      },
      plant_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting barley because ...
         descname = _("planting barley"),
         actions = {
            -- time of worker: 21.97-30.55 sec, min. time for 12 fields 21.6 sec
            -- min. time (12 fields): 21.6  + 10 = 31.6  sec
            -- min. time:             21.97 + 10 = 31.97 sec
            -- max. time:             30.55 + 10 = 40.55 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting barley because ...
         descname = _("harvesting barley"),
         actions = {
            -- time of worker: 23.97-32.55 sec, min. time for 12 fields 23.6 sec
            -- min. time (12 fields): 23.6  + 40 + 4 + 3.6 = 71.2  sec
            -- min. time:             23.97 + 40 + 4 + 3.6 = 71.57 sec
            -- max. time:             32.55 + 40 + 4 + 3.6 = 80.15 sec
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
