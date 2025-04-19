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
            -- the longest possible transition from tiny to ripe field:
            --   2 * (30 + 45 + 50) = 250 sec
            --   see data/tribes/immovables/cornfield/ and src/logic/map_objects/immovable_program.cc - ImmovableProgram::ActAnimate::execute()
            -- how many fields are needed for 100% productivity in worst case (nearest fields, longest field transition):
            --   1 + (250 - (6 + 2 * 1.8 + 10)) / 54.4 = 5.235
            -- calculation of productivity with 4 fields:
            --   irwinhall(8, scale=1/8).cdf(4/5.235) = 0.996
            --   using SciPy 1.14+
            -- min. time total (free 4 nearest fields):  29.2 + 25.2 = 54.4 sec
            -- max. time total (free 4 furthest fields): 40   + 36   = 76   sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
         descname = _("planting corn"),
         actions = {
            -- time of worker: 19.2-30 sec
            -- min. time: 19.2 + 10 = 29.2 sec
            -- max. time: 30   + 10 = 40   sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
         descname = _("harvesting corn"),
         actions = {
            -- time of worker: 21.2-32 sec
            -- min. time: 21.2 + 4 = 25.2 sec
            -- max. time: 32   + 4 = 36   sec
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
