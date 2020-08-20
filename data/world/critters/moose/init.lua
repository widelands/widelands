world:new_critter_type{
   name = "moose",
   descname = _ "Moose",
   animation_directory = path.dirname(__file__),
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   size = 10,
   reproduction_rate = 30,
   appetite = 50,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 15, 27 },
         fps = 20,
         sound_effect = {
            path = "sound/animals/moose",
            priority = 5
         },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 15, 27 },
         fps = 20,
      },
      walk = {
         hotspot = {21, 34},
         fps = 4,
         directional = true
      }
   }
}
