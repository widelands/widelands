dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 1, 10 },
      fps = 20,
   },
}

add_walking_animations(animations, "walk", dirname, "walk", {15, 25}, 20)

world:new_critter_type{
   name = "deer",
   descname = _ "Deer",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
