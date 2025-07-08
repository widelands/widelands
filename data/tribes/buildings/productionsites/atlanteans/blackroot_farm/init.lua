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
            -- nominal transition from tiny to ripe field (the longest possible transition is double):
            --   30 + 45 + 50 = 125 sec
            --   see data/tribes/immovables/blackrootfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 1):
            --   2 + (125 - (9 + 2 * 1.8 + 10)) / 59.4 = 3.72
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total: 35.2 + 24.2 = 59.4 sec
            -- max. time total: 49.6 + 38.6 = 88.2 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting blackroot because ...
         descname = _("planting blackroot"),
         actions = {
            -- time of worker: 25.2-39.6 sec
            -- min. time: 25.2 + 10 = 35.2 sec
            -- max. time: 39.6 + 10 = 49.6 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting blackroot because ...
         descname = _("harvesting blackroot"),
         actions = {
            -- time of worker: 21.2-35.6 sec
            -- min. time: 21.2 + 3 = 24.2 sec
            -- max. time: 35.6 + 3 = 38.6 sec
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
