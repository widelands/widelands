dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 13 },
      fps = 20,
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 11, 13 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {11, 20}, 20)

world:new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 5,
   reproduction_rate = 60,
   appetite = 20,
   herbivore = {"field"},
}
