dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 4, 9 },
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 4, 9 },
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {5, 9}, 4)

world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   reproduction_rate = 100,
   appetite = 100,
   herbivore = {"field"},
}
