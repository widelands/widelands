push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smokery_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Smokery"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_smokery_advanced",
        enhancement_cost = {
          marble_column = 2,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 2,
          quartz = 1,
          diamond = 1
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 53, 68 },
         fps = 20
      }
   },

   aihints = {
      prohibited_till = 3600
   },

   working_positions = {
      europeans_smoker_normal = 1
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
            "animate=working duration:25s",
            "sleep=duration:25s",
            "produce=smoked_fish"
         }
      },
      smoke_meat_basic = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            "consume=meat log",
            "animate=working duration:25s",
            "sleep=duration:25s",
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
            "animate=working duration:25s",
            "sleep=duration:25s",
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
            "animate=working duration:25s",
            "sleep=duration:25s",
            "produce=smoked_fish:3"
         }
      },
   },
}

pop_textdomain()
