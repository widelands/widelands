push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_recycling_center",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Recycling Center"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 3,
      granite = 2,
      log = 2,
      reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 70},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working_metal = {
         directory = dirname,
         basename = "working_metal",
         hotspot = {50, 71}, -- the whole animation is one pixel lower
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working_fur = {
         directory = dirname,
         basename = "working_fur",
         hotspot = {50, 70},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = {50, 58}
      }
   },

   aihints = {
      prohibited_till = 1200,
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      frisians_smelter = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "scrap_iron", amount = 8 },
      { name = "scrap_metal_mixed", amount = 8 },
      { name = "fur_garment_old", amount = 8 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smelt_iron",
            "call=make_fur",
            "call=smelt_iron",
            "call=smelt_mixed",
            "call=smelt_iron",
            "call=make_fur",
         }
      },
      make_fur = {
         -- TRANSLATORS: Completed/Skipped/Did not start recycling fur because ...
         descname = pgettext("frisians_building", "recycling fur"),
         actions = {
            "return=skipped unless site has fur_garment_old",
            "consume=fur_garment_old",
            "sleep=duration:40s",
            "animate=working_fur duration:15s",
            "produce=fur"
         }
      },
      smelt_iron = {
         -- TRANSLATORS: Completed/Skipped/Did not start recycling iron because ...
         descname = pgettext("frisians_building", "recycling iron"),
         actions = {
            "return=skipped unless economy needs iron or not economy needs coal", -- if the economy doesn't need coal the situation gets even improved because recycling saves coal
            "consume=scrap_iron:2 coal",
            "sleep=duration:40s",
            "animate=working_metal duration:40s",
            "produce=iron:2"
         }
      },
      smelt_mixed = {
         -- TRANSLATORS: Completed/Skipped/Did not start recycling iron and gold because ...
         descname = pgettext("frisians_building", "recycling iron and gold"),
         actions = {
            "return=skipped unless economy needs iron or economy needs gold or not economy needs coal", -- if the economy doesn't need coal the situation gets even improved because recycling saves coal
            "consume=scrap_metal_mixed:2 coal",
            "sleep=duration:40s",
            "animate=working_metal duration:40s",
            "produce=iron gold"
         }
      },
   }
}

pop_textdomain()
