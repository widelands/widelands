world:new_critter_type{
   name = "stag",
   descname = _ "Stag",
   animation_directory = path.dirname(__file__),
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   size = 7,
   reproduction_rate = 30,
   appetite = 30,
   herbivore = {"field"},

   animations = {
      idle = {
         sound_effect = {
            path = "sound/animals/stag",
            priority = 2
         },
         hotspot = { 12, 26 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 12, 26 },
         fps = 20,
      },
      walk = {
         hotspot = { 25, 30 },
         fps = 20,
         directional = true
      }
   }
}
