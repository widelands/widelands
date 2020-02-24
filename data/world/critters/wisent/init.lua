dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 14, 27 },
      fps = 20,
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = {14, 27},
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {24, 32}, 20)

world:new_critter_type{
   name = "wisent",
   descname = _ "Wisent",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 10,
   reproduction_rate = 20,
   appetite = 50,
   herbivore = {"field"},
}
