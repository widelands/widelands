world:new_critter_type{
   name = "wolf",
   descname = _ "Wolf",
   animation_directory = path.dirname(__file__),
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   size = 5,
   reproduction_rate = 80,
   appetite = 100,
   carnivore = true,

   animations = {
      idle = {
         hotspot = { 8, 15 },
         fps = 10,
         sound_effect = {
            -- Sound files with numbers starting from 10 are generating silence.
            path = "sound/animals/wolf",
            priority = 3
         },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 8, 15 },
         fps = 10,
      },
      walk = {
         hotspot = {19, 19},
         fps = 20,
         directional = true
      }
   }
}
