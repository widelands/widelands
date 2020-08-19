world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   animation_directory = path.dirname(__file__),
   programs = {
      remove = { "remove" },
   },
   size = 1,
   reproduction_rate = 100,
   appetite = 100,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 4, 9 },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 4, 9 },
      },
      walk = {
         hotspot = { 5, 9 },
         fps = 4,
         directional = true
      }
   }
}
