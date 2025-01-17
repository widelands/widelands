push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_reed_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Reed Farm"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 2,
      --granite = 1,
      log = 1,
      reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   spritesheets = {
      idle = {
         hotspot = {40, 72},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {40, 54}
      }
   },

   aihints = {
      space_consumer = true,
      very_weak_ai_limit = 2,
      weak_ai_limit = 3,
      basic_amount = 2,
   },

   working_positions = {
      frisians_reed_farmer = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- the longest possible transition from tiny to ripe field:
            --   2 * (22 + 28 + 40) = 180 sec
            --   see data/tribes/immovables/reedfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for 100% productivity in worst case (nearest fields, longest field transition):
            --   1 + (180 - (2.333 * 1.8 + 18)) / 54.8 = 3.880
            -- calculation of productivity with 3 fields:
            --   irwinhall(6, scale=1/6).cdf(3/3.880) = 0.991
            --   using SciPy 1.14+
            -- min. time total (free 3 nearest fields):  29.4 + 25.4 = 54.8 sec
            -- max. time total (free 3 furthest fields): 33   + 29   = 62   sec
            "call=plant_reed",
            "call=harvest_reed",
         }
      },
      plant_reed = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
         descname = _("planting reed"),
         actions = {
            -- time of worker: 11.4-15 sec
            -- min. time: 11.4 + 18 = 29.4 sec
            -- max. time: 15   + 18 = 33   sec
            "callworker=plantreed",
            "sleep=duration:18s"
         }
      },
      harvest_reed = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
         descname = _("harvesting reed"),
         actions = {
            -- time of worker: 20.4-24 sec
            -- min. time: 20.4 + 5 = 25.4 sec
            -- max. time: 24   + 5 = 29   sec
            "callworker=harvestreed",
            "sleep=duration:5s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Fields"),
      heading = _("Out of Fields"),
      message = pgettext("frisians_building", "The reed farmer working at this reed farm has no cleared soil to plant his seeds."),
      productivity_threshold = 20
   },
}

pop_textdomain()
