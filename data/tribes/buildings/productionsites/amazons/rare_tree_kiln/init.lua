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
      unoccupied = {hotspot = {45, 50}},
      idle = {hotspot = {45, 50}},
   },
   spritesheets = {
      working = {
         hotspot = {45, 50},
         fps = 15,
         frames = 20,
         columns = 10,
         rows = 2
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
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing charcoal because ...
         descname = _("producing charcoal"),
         actions = {
            "return=skipped unless economy needs coal",
            "return=skipped unless site has ironwood",
            "return=skipped unless site has balsa",
            "return=skipped unless site has rubber",
            "consume=ironwood balsa rubber",
            "sleep=duration:30s",
            "animate=working duration:30s",
            "produce=coal"
         },
      },
   },
}

pop_textdomain()
