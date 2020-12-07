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
      prohibited_till = 1800
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
            "call=smoke_fish_basic",
            "call=smoke_meat_basic",
            "call=smoke_fish",
            "call=smoke_meat",
         }
      },
      smoke_fish_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            "consume=fish log",
            "animate=working duration:60s",
            "sleep=duration:60s",
            "produce=smoked_fish"
         }
      },
      smoke_meat_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            "return=skipped unless economy needs smoked_meat",
            "consume=meat log",
            "animate=working duration:60s",
            "sleep=duration:60s",
            "produce=smoked_meat"
         }
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            "return=skipped unless economy needs smoked_meat",
            "return=skipped when economy needs log",
            "consume=meat:3 log:2",
            "animate=working duration:60s",
            "sleep=duration:60s",
            "produce=smoked_meat:3"
         }
      },
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            "return=skipped unless economy needs smoked_fish",
            "return=skipped when economy needs log",
            "consume=fish:3 log:2",
            "animate=working duration:60s",
            "sleep=duration:60s",
            "produce=smoked_fish:3"
         }
      },
   },
}

pop_textdomain()
