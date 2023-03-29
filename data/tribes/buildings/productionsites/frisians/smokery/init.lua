push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_smokery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Smokery"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 2,
      granite = 2,
      log = 1,
      reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      reed = 1
   },

   spritesheets = {
      idle = {
         hotspot = {50, 71},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      },
      working = {
         hotspot = {50, 71},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      unoccupied = {
         hotspot = {50, 58}
      }
   },

   aihints = {
      prohibited_till = 470,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      frisians_smoker = 1
   },

   inputs = {
      { name = "meat", amount = 6 },
      { name = "fish", amount = 6 },
      { name = "log", amount = 6 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=smoke_meat",
            "call=smoke_fish",
         },
      },
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _("smoking fish"),
         actions = {
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "sleep=duration:16s",
            "animate=working duration:30s",
            "produce=smoked_fish:2"
         },
      },
      smoke_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _("smoking meat"),
         actions = {
            "return=skipped when site has fish:2 and economy needs smoked_fish",
            "return=skipped unless economy needs smoked_meat",
            "consume=meat:2 log",
            "sleep=duration:16s",
            "animate=working duration:30s",
            "produce=smoked_meat:2"
         },
      },
   },
}

pop_textdomain()
