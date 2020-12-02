push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_smokery_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Smokery"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 52, 58 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 52, 58 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_smoker_advanced = 1
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
            -- time total: 40
            "return=skipped unless economy needs smoked_meat",
            "consume=meat:2 log",
            "animate=working duration:20s",
            "sleep=duration:20s",
            "produce=smoked_meat:2"
         }
      },
      -- 2 identical programs for fish to prevent unnecessary skipping penalty
      smoke_fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 40
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "animate=working duration:20s",
            "sleep=duration:20s",
            "produce=smoked_fish:2"
         }
      },
      smoke_fish_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
         descname = _"smoking fish",
         actions = {
            -- time total: 40
            "return=skipped unless economy needs smoked_fish",
            "consume=fish:2 log",
            "animate=working duration:20s",
            "sleep=duration:20s",
            "produce=smoked_fish:2"
         }
      },
      smoke_meat_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
         descname = _"smoking meat",
         actions = {
            -- time total: 50
            "return=skipped unless economy needs smoked_meat",
            "consume=meat:2 log",
            "animate=working duration:20s",
            "sleep=duration:20s",
            "produce=smoked_meat:2"
         }
      },
   },
}

pop_textdomain()
