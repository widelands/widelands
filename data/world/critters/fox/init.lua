dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         path = "sound/animals/coyote",
         priority = 0.01
      },
      hotspot = { 10, 13 },
      fps = 10,
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 10, 13 },
      fps = 10,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {11, 14}, 20)

world:new_critter_type{
   name = "fox",
   descname = _ "Fox",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 4,
   reproduction_rate = 80,
   appetite = 70,
   carnivore = true,
}
