dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         directory = dirname,
         name = "duck",
      },
      hotspot = { 5, 7 },
      fps = 4,
   },
}

add_walking_animations(animations, "walk", dirname, "walk", {5, 10})

world:new_critter_type{
   name = "duck",
   descname = _ "Duck",
   editor_category = "critters_aquatic",
   attributes = { "swimming" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
