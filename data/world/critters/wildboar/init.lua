dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 18 },
      fps = 20,
      sound_effect = {
         path = "sound/animals/boar",
         priority = 0.01
      },
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 10, 18 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {20, 22}, 20)

world:new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 8,
   reproduction_rate = 10,
   appetite = 20,
   herbivore = {"field"},
}
