push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_cassava_plantation",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Cassava Plantation"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 4,
      rope = 2,
      granite = 2,
   },
   return_on_dismantle = {
      log = 2,
      rope = 1,
      granite = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {92, 73}},
      unoccupied = {hotspot = {92, 73}},
   },

   aihints = {
      space_consumer = true,
      prohibited_till = 220,
   },

   working_positions = {
      amazons_cassava_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- nominal transition from tiny to ripe field (the longest possible transition is double):
            --   30 + 45 + 50 = 125 sec
            --   see data/tribes/immovables/cassavafield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 1):
            --   2 + (125 - (6 + 2 * 1.8 + 10)) / 54.4 = 3.94
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total (free 4 nearest fields):  29.2 + 25.2 = 54.4 sec
            -- max. time total (free 4 furthest fields): 44.5 + 40.5 = 85   sec
            "call=plant_cassava",
            "call=harvest_cassava",
         }
      },
      plant_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting cassava because ...
         descname = _("planting cassava"),
         actions = {
            -- time of worker: 19.2-34.5 sec
            -- min. time: 19.2 + 10 = 29.2 sec
            -- max. time: 34.5 + 10 = 44.5 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting cassava because ...
         descname = _("harvesting cassava"),
         actions = {
            -- time of worker: 21.2-36.5 sec
            -- min. time: 21.2 + 4 = 25.2 sec
            -- max. time: 36.5 + 4 = 40.5 sec
            "callworker=harvest",
            "sleep=duration:4s",
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext ("amazons_building", "The farmer working at this cassava plantation has no cleared soil to plant her seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
