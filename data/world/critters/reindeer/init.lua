world:new_critter_type{
   name = "reindeer",
   descname = _ "Reindeer",
   animation_directory = path.dirname(__file__),
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   size = 4,
   reproduction_rate = 60,
   appetite = 60,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 23, 21 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 23, 21 },
         fps = 20,
      },
      walk = {
         hotspot = { 25, 30 },
         fps = 10,
         directional = true
      }
   }
}
