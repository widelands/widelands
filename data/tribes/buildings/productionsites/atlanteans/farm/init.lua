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
            -- std time total (free all fields except road): 34.186 + 30.186 = 64.372 sec
            -- ideal time total (free 5 nearest fields): 29.92 + 25.92 = 55.84 sec
            "call=plant",
            "call=harvest",
         }
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
         descname = _("planting corn"),
         actions = {
            -- time of worker: std case 24.186 sec, ideal case 19.92 sec
            -- std time: 24.186 + 10 = 34.186 sec
            -- ideal time: 19.92 + 10 = 29.92 sec
            "callworker=plant",
            "sleep=duration:10s"
         }
      },
      harvest = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
         descname = _("harvesting corn"),
         actions = {
            -- time of worker: std case 26.186 sec, ideal case 21.92 sec
            -- std time: 26.186 + 4 = 30.186 sec
            -- ideal time: 21.92 + 4 = 25.92 sec
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
