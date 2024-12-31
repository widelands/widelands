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
            -- longest possible transition from tiny to ripe field:
            --   2 * (22 + 28 + 40) = 180 sec
            --   see data/tribes/immovables/reedfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for reliable 100% productivity: 4
            --   with 3 fields, this inequation is not fulfilled:
            --   (180 + 54.8 - (6.5 + 2.333 * 1.8 + 8)) / 54.8 < 3
            -- min. time total (free 3 nearest fields):  29.4 + 25.4 = 54.8 sec
            -- min. time total (free 4 nearest fields):  30   + 26   = 56   sec
            -- max. time total (free 4 furthest fields): 31.8 + 27.8 = 59.6 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting reed"),
         actions = {
            -- time of worker: 22-23.8 sec, min. time for 3 fields 21.4 sec
            -- min. time (3 fields): 21.4 + 8 = 29.4 sec
            -- min. time:            22   + 8 = 30   sec
            -- max. time:            23.8 + 8 = 31.8 sec
            "callworker=plant",
            "sleep=duration:8s" -- orig sleep=duration:20s but gardener animation was increased by 2sec
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting reed"),
         actions = {
            -- time of worker: 23-24.8 sec, min. time for 3 fields 22.4 sec
            -- min. time (3 fields): 22.4 + 3 = 25.4 sec
            -- min. time:            23   + 3 = 26   sec
            -- max. time:            24.8 + 3 = 27.8 sec
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
