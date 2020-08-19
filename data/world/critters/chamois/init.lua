world:new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   animation_directory = path.dirname(__file__),
   editor_category = "critters_herbivores",
   programs = {
      remove = { "remove" },
   },
   size = 5,
   reproduction_rate = 60,
   appetite = 20,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 11, 13 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 11, 13 },
         fps = 20,
      },
      walk = {
         hotspot = { 11, 20 },
         fps = 20,
         directional = true
      }
   }

}
