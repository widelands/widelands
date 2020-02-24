dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = {21, 16},
      fps = 20
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 21, 16 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {24, 24}, 4)

world:new_critter_type{
   name = "brownbear",
   descname = _ "Brown bear",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   reproduction_rate = 60,
   appetite = 90,
   carnivore = { "sheep", "chamois", "deer", "stag", "reindeer" },
}
