push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_charcoal_kiln_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Charcoal Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
        name = "europeans_charcoal_kiln_advanced",
        enhancement_cost = {
          planks = 2,
          brick = 2,
          grout = 2,
          spidercloth = 2,
          quartz = 1
        },
        enhancement_return_on_dismantle = {
          granite = 2,
          quartz = 1,
        },
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 57 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 47, 60 },
      },
   },

   aihints = {
   },

   working_positions = {
      europeans_charcoal_burner_normal = 1
   },

   inputs = {
      { name = "log", amount = 16 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
         descname = _"producing coal",
         actions = {
            "return=skipped unless economy needs coal",
            "return=skipped when economy needs log",
            "consume=log:8",
            "sleep=duration:30s",
            "animate=working duration:2m00s", -- Charcoal fires will burn for some days in real life
            "produce=coal:3"
         }
      },

   },
}

pop_textdomain()
