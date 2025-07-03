push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "barbarians_reed_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Reed Yard"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 5,
      granite = 2
   },
   return_on_dismantle = {
      log = 2,
      granite = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 46, 44 },
      },
   },

   aihints = {
      space_consumer = true,
      basic_amount = 1
   },

   working_positions = {
      barbarians_gardener = 1
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
            --   2 + (90 - (6.5 + 2.333 * 1.8 + 8)) / 54.8 = 3.30
            -- procedure how to make verification measurement:
            --   https://codeberg.org/wl/widelands/pulls/4725#issuecomment-4062998
            -- min. time total (avg. for 3 nearest fields): 29.4 + 25.4 = 54.8 sec
            -- min. time total:                             28.2 + 24.2 = 52.4 sec
            -- max. time total:                             35.4 + 31.4 = 66.8 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting reed"),
         actions = {
            -- time of worker: 20.2-27.4 sec
            -- min. time: 20.2 + 8 = 28.2 sec
            -- max. time: 27.4 + 8 = 35.4 sec
            "callworker=plant",
            "sleep=duration:8s" -- orig sleep=duration:20s but gardener animation was increased by 2sec
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting reed"),
         actions = {
            -- time of worker: 21.2-28.4 sec
            -- min. time: 21.2 + 3 = 24.2 sec
            -- max. time: 28.4 + 3 = 31.4 sec
            "callworker=harvest",
            "sleep=duration:3s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("barbarians_building", "The gardener working at this reed yard has no cleared soil to plant his seeds."),
      productivity_threshold = 20
   },
}

pop_textdomain()
