push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_stonemasons_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Stonemason’s House"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 61 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 58, 61 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_stonemason_normal = 1
   },

   inputs = {
      { name = "marble", amount = 6 },
      { name = "granite", amount = 8 },
      { name = "clay", amount = 6 },
      { name = "water", amount = 6 },
      { name = "coal", amount = 6 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=mixing_grout",
            "call=burning_bricks",
            "call=sculpting_marble_column",
         }
      },
      mixing_grout = {
         -- TRANSLATORS: Completed/Skipped/Did not start mixing grout because ...
         descname = _"mixing grout",
         actions = {
            "return=skipped unless economy needs grout",
            "consume=coal:2 granite:3 water:3",
            "sleep=duration:30s",
            "playsound=sound/barbarians/stonegrind priority:80%",
            "animate=working duration:30s",
            "playsound=sound/barbarians/mortar priority:60%",
            "sleep=duration:3s",
            "produce=grout:3"
         }
      },
      burning_bricks = {
         -- TRANSLATORS: Completed/Skipped/Did not start burning bricks because ...
         descname = _"burning bricks",
         actions = {
            "return=skipped unless economy needs brick",
            "consume=coal granite clay:3",
            "sleep=duration:30s",
            "playsound=sound/barbarians/stonegrind priority:80%",
            "animate=working duration:30s",
            "sleep=duration:3s",
            "produce=brick:3"
         },
      },
      sculpting_marble_column = {
         -- TRANSLATORS: Completed/Skipped/Did not start sculpting a marble column because ...
         descname = _"sculpting a marble column",
         actions = {
            "return=skipped unless economy needs marble_column",
            "consume=marble:3",
            "sleep=duration:30s",
            "playsound=sound/stonecutting/stonemason priority:50% allow_multiple",
            "animate=working duration:30s",
            "produce=marble_column:2"
         }
      }
   },
}

pop_textdomain()
