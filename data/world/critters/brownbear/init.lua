wl.World():new_critter_type{
   name = "brownbear",
   descname = _ "Brown bear",
   animation_directory = path.dirname(__file__),
   programs = {
      remove = { "remove" },
   },
   size = 10,
   reproduction_rate = 60,
   appetite = 90,
   carnivore = true,

   animations = {
      idle = {
         hotspot = { 21, 16 },
         fps = 20
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 21, 16 },
         fps = 20,
      },
      walk = {
         hotspot = { 24, 24 },
         fps = 4,
         directional = true
      }
   }
}
