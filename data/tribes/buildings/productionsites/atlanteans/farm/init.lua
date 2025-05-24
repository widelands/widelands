push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 3,
      granite = 2,
      planks = 2,
      spidercloth = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      planks = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 86, 78 },
      },
   },

   aihints = {
      space_consumer = true,
      basic_amount = 1,
       -- Farm needs spidercloth to be built and spidercloth needs corn for production
       -- -> farm should be built ASAP!
      prohibited_till = 250,
      very_weak_ai_limit = 1,
      weak_ai_limit = 3
   },

   working_positions = {
      atlanteans_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- nominal transition from tiny to ripe field (the longest possible transition is double):
            --   30 + 45 + 50 = 125 sec
            --   see data/tribes/immovables/cornfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- estimation of number of nearest fields for optimal productivity (nominal + 1):
            --   2 + (125 - (6 + 2 * 1.8 + 10)) / 54.4 = 3.94
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total: 29.2 + 25.2 = 54.4 sec
            -- max. time total: 43.6 + 39.6 = 83.2 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
         descname = _("planting corn"),
         actions = {
            -- time of worker: 19.2-33.6 sec
            -- min. time: 19.2 + 10 = 29.2 sec
            -- max. time: 33.6 + 10 = 43.6 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
         descname = _("harvesting corn"),
         actions = {
            -- time of worker: 21.2-35.6 sec
            -- min. time: 21.2 + 4 = 25.2 sec
            -- max. time: 35.6 + 4 = 39.6 sec
            "callworker=harvest",
            "sleep=duration:4s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("atlanteans_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
