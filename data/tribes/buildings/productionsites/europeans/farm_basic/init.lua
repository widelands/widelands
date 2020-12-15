push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_farm_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
        name = "europeans_farm_level_1",
        enhancement_cost = {
          brick = 3,
          grout = 3,
          spidercloth = 3,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 3,
          quartz = 1,
        },
   },

   buildcost = {
      planks = 4,
      reed = 4,
      granite = 2
   },
   return_on_dismantle = {
      log = 4,
      granite = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 69, 76 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 69, 76 },
      },
      unoccupied = {
         pictures = path.list_files(dirname .. "unoccupied_??.png"),
         hotspot = { 69, 76 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 69, 76 },
      },
   },

   aihints = {
      space_consumer = true,
      supports_production_of = { "honey" },
      prohibited_till = 900
   },

   working_positions = {
      europeans_farmer_basic = 3
   },
   
   inputs = {
      { name = "water", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant_corn_basic",
            "call=plant_corn",
            "call=plant_wheat",
            "call=harvest_corn",
            "call=plant_rye",
            "call=plant_barley",
            "call=harvest_wheat",
            "call=harvest_rye",
            "call=harvest_barley",
            "call=harvest_corn_basic",
         }
      },
      plant_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting barley because ...
         descname = _"planting barley",
         actions = {
            "return=skipped unless economy needs barley",
            "return=skipped when economy needs water",
            "return=skipped when economy needs corn",
            "return=skipped when economy needs rye",
            "return=skipped when economy needs wheat",
            "consume=water",
            "callworker=plant_barley",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      plant_corn_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
         descname = _"planting corn",
         actions = {
            "consume=water",
            "callworker=plant_corn",
            "animate=working duration:5s",
            "sleep=duration:5s"
         }
      },
      plant_corn = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
         descname = _"planting corn",
         actions = {
            "return=skipped unless economy needs corn",
            "return=skipped when economy needs water",
            "return=skipped when economy needs barley",
            "return=skipped when economy needs rye",
            "return=skipped when economy needs wheat",
            "consume=water",
            "callworker=plant_corn",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      plant_rye = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting rye because ...
         descname = _"planting rye",
         actions = {
            "return=skipped unless economy needs rye",
            "return=skipped when economy needs water",
            "return=skipped when economy needs barley",
            "return=skipped when economy needs corn",
            "return=skipped when economy needs wheat",
            "consume=water",
            "callworker=plant_rye",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      plant_wheat = {
         -- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
         descname = _"planting wheat",
         actions = {
            "return=skipped unless economy needs wheat",
            "return=skipped when economy needs water",
            "return=skipped when economy needs barley",
            "return=skipped when economy needs corn",
            "return=skipped when economy needs rye",
            "consume=water",
            "callworker=plant_wheat",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      harvest_barley = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting barley because ...
         descname = _"harvesting barley",
         actions = {
            "return=skipped unless economy needs barley",
            "return=skipped when economy needs corn",
            "return=skipped when economy needs rye",
            "return=skipped when economy needs wheat",
            "callworker=harvest_barley",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      harvest_corn_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
         descname = _"harvesting corn",
         actions = {
            "callworker=harvest_corn",
            "animate=working duration:5s",
            "sleep=duration:5s"
         }
      },
      harvest_corn = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
         descname = _"harvesting corn",
         actions = {
            "return=skipped unless economy needs corn",
            "return=skipped when economy needs barley",
            "return=skipped when economy needs rye",
            "return=skipped when economy needs wheat",
            "callworker=harvest_corn",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      harvest_rye = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting rye because ...
         descname = _"harvesting rye",
         actions = {
            "return=skipped unless economy needs rye",
            "return=skipped when economy needs barley",
            "return=skipped when economy needs corn",
            "return=skipped when economy needs wheat",
            "callworker=harvest_rye",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
      harvest_wheat = {
         -- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
         descname = _"harvesting wheat",
         actions = {
            "return=skipped unless economy needs wheat",
            "return=skipped when economy needs barley",
            "return=skipped when economy needs corn",
            "return=skipped when economy needs rye",
            "callworker=harvest_wheat",
            "animate=working duration:6s",
            "sleep=duration:5s"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Fields",
      heading = _"Out of Fields",
      message = pgettext("barbarians_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
      productivity_threshold = 30
   },
}

pop_textdomain()
