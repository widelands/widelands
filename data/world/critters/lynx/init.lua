push_textdomain("world")

wl.World():new_critter_type{
   name = "lynx",
   descname = _ "Lynx",
   animation_directory = path.dirname(__file__),
   programs = {
      remove = { "remove" },
   },
   size = 2,
   reproduction_rate = 30,
   appetite = 10,
   carnivore = true,

   animations = {
      idle = {
         hotspot = { 8, 14 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 8, 14 },
         fps = 20,
      },
      walk = {
         hotspot = { 11, 21 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
