world:new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   animation_directory = path.dirname(__file__),
   editor_category = "critters_herbivores",
   programs = {
      remove = { "remove" },
   },
   size = 8,
   reproduction_rate = 10,
   appetite = 20,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 10, 18 },
         fps = 20,
         sound_effect = {
            path = "sound/animals/boar",
            priority = 0.01
         },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 10, 18 },
         fps = 20,
      },
      walk = {
         hotspot = { 20, 22 },
         fps = 20,
         directional = true
      }
   }
}
