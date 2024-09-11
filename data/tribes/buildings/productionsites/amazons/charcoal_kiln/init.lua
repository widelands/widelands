push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "amazons_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Charcoal Kiln"),
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement = {
      name = "amazons_rare_tree_kiln",
      enhancement_cost = {
         granite = 3,
         quartz = 1,
      },
      enhancement_return_on_dismantle = {
         quartz = 1,
      }
   },

   buildcost = {
      log = 2,
      granite = 4,
      ironwood = 1,
   },
   return_on_dismantle = {
      granite = 3,
   },

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
      amazons_charcoal_burner = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing charcoal because ...
         descname = _("producing charcoal"),
         actions = {
            -- time total: 15.4 + 85 + 3.6 = 104 sec
            "return=skipped unless economy needs coal",
            -- Make it a little more efficient than other tribes
            "consume=log:5",
            "sleep=duration:15s400ms",
            "animate=working duration:85s", -- Charcoal fires will burn for some days in real life
            "produce=coal"
         }
      },
   },
}

pop_textdomain()
