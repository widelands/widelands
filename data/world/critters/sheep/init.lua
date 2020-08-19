world:new_critter_type{
   name = "sheep",
   descname = _ "Sheep",
   animation_directory = path.dirname(__file__),
   programs = {
      remove = { "remove" },
   },
   size = 3,
   reproduction_rate = 40,
   appetite = 90,
   herbivore = {"field"},

   animations = {
      idle = {
         sound_effect = {
            path = "sound/farm/sheep",
            priority = 0.01
         },
         hotspot = { 8, 16 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 8, 16 },
         fps = 20,
      },
      walk = {
         hotspot = { 15, 25 },
         fps = 20,
         directional = true
      }
   }
}
