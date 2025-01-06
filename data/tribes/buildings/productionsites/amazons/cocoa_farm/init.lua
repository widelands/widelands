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
   spritesheets = { idle = {
      hotspot = {92, 112},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }},

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
            -- longest possible transition from sapling to old tree:
            --   2 * (80 + 60 + 60) = 400 sec
            --   in each tree stage, it may disappear with certain small probability
            --   see data/tribes/immovables/trees/cocoa/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for 100% productivity (not counting on the disappearance of trees): 8
            --   with 7 fields, this inequation is not fulfilled:
            --   (400 + 57.48 - (6 + 2.429 * 1.8 + 10)) / 57.48 < 7
            -- min. time total (free 7 nearest fields):  30.74 + 26.74 = 57.48 sec
            -- min. time total (free 8 nearest fields):  31    + 27    = 58    sec
            -- max. time total (free 8 furthest fields): 42.25 + 38.25 = 80.5  sec
            "call=plant_cocoa",
            "call=harvest_cocoa",
         }
      },
      plant_cocoa = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting cocoa"),
         actions = {
            -- time of worker: 21-32.25 sec, min. time for 7 fields 20.74 sec
            -- min. time (7 fields): 20.74 + 10 = 30.74 sec
            -- min. time:            21    + 10 = 31    sec
            -- max. time:            32.25 + 10 = 42.25 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cocoa = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting cocoa"),
         actions = {
            -- time of worker: 23-34.25 sec, min. time for 7 fields 22.74 sec
            -- min. time (7 fields): 22.74 + 4 = 26.74 sec
            -- min. time:            23    + 4 = 27    sec
            -- max. time:            34.25 + 4 = 38.25 sec
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
