push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_rare_tree_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Rare Tree Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   animation_directory = dirname,
   animations = {
      unoccupied = {hotspot = {43, 44}},
      idle = {hotspot = {43, 44}},
   },
   spritesheets = {
      working = {
         hotspot = {43, 45},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      }
   },

   aihints = {
   },

   working_positions = {
      amazons_charcoal_master = 1
   },

   inputs = {
      { name = "ironwood", amount = 3 },
      { name = "balsa", amount = 3 },
      { name = "rubber", amount = 3 },
      { name = "granite", amount = 2 },
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing charcoal because ...
         descname = _("producing charcoal"),
         actions = {
            "return=skipped unless economy needs coal",
            "return=skipped unless site has granite",
            "return=skipped unless site has ironwood",
            "return=skipped unless site has balsa",
            "return=skipped unless site has rubber",
            "call=repair",
            -- first one has less sleep time to compensate for 10s delay on skipping
            -- average time without repair is always 60s, similar to Frisian charcoal kiln
            "call=burn_first",
            "call=burn",
            "call=burn",
         },
      },
      repair = {
         descname = _("repairing kiln"),
         actions = {
            "return=failed unless site has granite",
            "consume=granite",
            "sleep=duration:18s",
         },
      },
      burn_first = {
         descname = _("producing charcoal"),
         actions = {
            "return=skipped unless site has ironwood",
            "return=skipped unless site has balsa",
            "return=skipped unless site has rubber",
            "consume=ironwood balsa rubber",
            "sleep=duration:10s",
            "animate=working duration:30s",
            "produce=coal"
         },
      },
      burn = {
         descname = _("producing charcoal"),
         actions = {
            "return=skipped unless site has ironwood",
            "return=skipped unless site has balsa",
            "return=skipped unless site has rubber",
            "consume=ironwood balsa rubber",
            "sleep=duration:40s",
            "animate=working duration:30s",
            "produce=coal"
         },
      },
      encyclopedia = {
         -- dummy program to fix encyclopedia
         descname = "encyclopedia",
         actions = {
            "consume=ironwood:3 balsa:3 rubber:3 granite",
            "produce=coal:3",
         },
      },
   },
}

pop_textdomain()
