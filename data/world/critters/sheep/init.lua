dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         path = "sound/farm/sheep",
         priority = 0.01
      },
      hotspot = { 8, 16 },
      fps = 20,
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 8, 16 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {15, 25}, 20)

world:new_critter_type{
   name = "sheep",
   descname = _ "Sheep",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 3,
   reproduction_rate = 40,
   appetite = 90,
   herbivore = {"field"},
}
