push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smokery_basic",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Basic Smokery"),
   icon = dirname .. "menu.png",
   size = "medium",
   
   enhancement = {
        name = "europeans_smokery_normal",
        enhancement_cost = {
          planks = 2,
          brick = 1,
          grout = 1,
          spidercloth = 2,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 1,
          quartz = 1,
        },
   },

   buildcost = {
      reed = 2,
      planks = 4,
      brick = 2,
      grout = 2
   },
   return_on_dismantle = {
      log = 4,
      granite = 2
   },

   animations = {
      unoccupied = {
         directory = dirname,
         basename = "unoccupied",
         hotspot = { 52, 64 },
      }
   },
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 50, 65 }
      },
      build = {
         directory = dirname,
         basename = "build",
         frames = 4,
         columns = 2,
         rows = 2,
         hotspot = { 50, 61 }
      },
      working = {
         directory = dirname,
         basename = "working",
         frames = 20,
         columns = 4,
         rows = 5,
         hotspot = { 53, 65 }
      }
   },

   aihints = {
   },

   working_positions = {
      europeans_smoker_basic = 1
   },

   inputs = {
      { name = "fish", amount = 4 },
      { name = "meat", amount = 4 },
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=smoke_fish",
            "call=smoke_meat",
            "call=smoke_fish_2",
            "call=smoke_meat_2",
         }
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_meat",
            "consume=meat:2 log",
            "animate=working duration:30s",
            "sleep=duration:30s",
            "produce=smoked_meat:2"
         }
      },
      -- 2 identical programs for fish to prevent unnecessary skipping penalty
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "animate=working duration:30s",
            "sleep=duration:30s",
            "produce=smoked_fish:2"
         }
      },
      smoke_fish_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "animate=working duration:30s",
            "sleep=duration:30s",
            "produce=smoked_fish:2"
         }
      },
      smoke_meat_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            -- time total: 60
            "return=skipped unless economy needs smoked_meat",
            "consume=meat:2 log",
            "animate=working duration:30s",
            "sleep=duration:30s",
            "produce=smoked_meat:2"
         }
      },
   },
}

pop_textdomain()
