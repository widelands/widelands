push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_productionsite_type {
   name = "frisians_honey_bread_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Honey Bread Bakery"),
   icon = dirname .. "menu.png",
   size = "medium",

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = {50, 82},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         directory = dirname,
         basename = "working",
         hotspot = {50, 82},
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
         hotspot = {50, 70}
      }
   },

   aihints = {},

   working_positions = {
      frisians_baker = 1,
      frisians_baker_master = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 },
      { name = "honey", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=bake_honey",
            "call=bake_normal",
            "call=bake_honey_2",
         }
      },
      -- 2 identical programs for honey bread to prevent unnecessary skipping penalty
      bake_honey = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking honey bread because ...
         descname = _"baking honey bread",
         actions = {
            "return=skipped unless economy needs honey_bread or workers need experience",
            "consume=barley water honey",
            "sleep=duration:20s",
            "animate=working duration:25s",
            "produce=honey_bread"
         }
      },
      bake_honey_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking honey bread because ...
         descname = _"baking honey bread",
         actions = {
            "return=skipped unless economy needs honey_bread or workers need experience",
            "consume=barley water honey",
            "sleep=duration:20s",
            "animate=working duration:25s",
            "produce=honey_bread"
         }
      },
      bake_normal = {
         -- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
         descname = _"baking bread",
         actions = {
            "return=skipped unless economy needs bread_frisians",
            "consume=barley water",
            "sleep=duration:20s",
            "animate=working duration:20s",
            "produce=bread_frisians"
         }
      },
   },
}

pop_textdomain()
