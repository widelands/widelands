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
            -- longest possible transition from tiny to ripe field:
            --   2 * (30 + 45 + 50) = 250 sec
            --   see data/tribes/immovables/cassavafield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for reliable 100% productivity: 6
            --   with 5 fields, this inequation is not fulfilled:
            --   (250 + 55.84 - (6 + 2.2 * 1.8 + 10)) / 55.84 < 5
            -- min. time total (free 5 nearest fields):  29.92 + 25.92 = 55.84 sec
            -- min. time total (free 6 nearest fields):  30.4  + 26.4  = 56.8  sec
            -- max. time total (free 6 furthest fields): 43    + 39    = 82    sec
            "call=plant_cassava",
            "call=harvest_cassava",
         }
      },
      plant_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting cassava because ...
         descname = _("planting cassava"),
         actions = {
            -- time of worker: 20.4-33 sec, min. time for 5 fields 19.92 sec
            -- min. time (5 fields): 19.92 + 10 = 29.92 sec
            -- min. time:            20.4  + 10 = 30.4  sec
            -- max. time:            33    + 10 = 43    sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest_cassava = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting cassava because ...
         descname = _("harvesting cassava"),
         actions = {
            -- time of worker: 22.4-35 sec, min. time for 5 fields 21.92 sec
            -- min. time (5 fields): 21.92 + 4 = 25.92 sec
            -- min. time:            22.4  + 4 = 26.4  sec
            -- max. time:            35    + 4 = 39    sec
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
