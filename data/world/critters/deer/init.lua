push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_critter_type{
   name = "deer",
   descname = _ "Deer",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 4,
   reproduction_rate = 40,
   appetite = 70,
   herbivore = {"tree_sapling"},

   animations = {
      idle = {
         hotspot = { 1, 10 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 1, 10 },
         fps = 20,
      },
      walk = {
         hotspot = { 15, 25 },
         fps = 20,
         directional = true
      }
   }

}

pop_textdomain()
