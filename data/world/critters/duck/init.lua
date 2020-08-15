dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         path = dirname .. "duck",
         priority = 0.01
      },
      hotspot = { 5, 7 },
      fps = 4,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {5, 10})

world:new_critter_type{
   name = "duck",
   descname = _ "Duck",
   editor_category = "critters_aquatic",
   attributes = { "swimming" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 1,
   reproduction_rate = 10,
}
