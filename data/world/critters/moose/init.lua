dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 15, 27 },
      fps = 20,
      sound_effect = {
         path = "sound/animals/moose",
         priority = 5
      },
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 15, 27 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {21, 34}, 20)

world:new_critter_type{
   name = "moose",
   descname = _ "Moose",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 10,
   reproduction_rate = 30,
   appetite = 50,
   herbivore = {"field"},
}
