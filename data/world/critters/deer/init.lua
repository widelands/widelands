dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 1, 10 },
      fps = 20,
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 1, 10 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {15, 25}, 20)

world:new_critter_type{
   name = "deer",
   descname = _ "Deer",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 4,
   reproduction_rate = 40,
   appetite = 70,
   herbivore = {"field"},
}
