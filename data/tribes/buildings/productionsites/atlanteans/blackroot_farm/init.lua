push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_blackroot_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Blackroot Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 3,
      granite = 2,
      log = 4
   },
   return_on_dismantle = {
      planks = 1,
      granite = 2,
      log = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 86, 78 },
      },
   },

   aihints = {
      prohibited_till = 550,
      space_consumer = true,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2
   },

   working_positions = {
      atlanteans_blackroot_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- longest possible transition from tiny to ripe field:
            --   2 * (30 + 45 + 50) = 250 sec
            --   see data/tribes/immovables/blackrootfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for reliable 100% productivity: 5
            --   with 5 fields, this inequation is not fulfilled:
            --   (250 + 59.4 - (9 + 2 * 1.8 + 10)) / 59.4 < 4
            -- min. time total (free 4 nearest fields):  35.2  + 24.2  = 59.4  sec
            -- min. time total (free 5 nearest fields):  35.92 + 24.92 = 60.84 sec
            -- max. time total (free 5 furthest fields): 45.28 + 34.28 = 79.56 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting blackroot because ...
         descname = _("planting blackroot"),
         actions = {
            -- time of worker: 25.92-35.28 sec, min. time for 4 fields 25.2 sec
            -- min. time (4 fields): 25.2  + 10 = 35.2  sec
            -- min. time:            25.92 + 10 = 35.92 sec
            -- max. time:            35.28 + 10 = 45.28 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting blackroot because ...
         descname = _("harvesting blackroot"),
         actions = {
            -- time of worker: 21.92-31.28 sec, min. time for 4 fields 21.2 sec
            -- min. time (4 fields): 21.2  + 3 = 24.2  sec
            -- min. time:            21.92 + 3 = 24.92 sec
            -- max. time:            31.28 + 3 = 34.28 sec
            "callworker=harvest",
            "sleep=duration:3s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("atlanteans_building", "The blackroot farmer working at this blackroot farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
