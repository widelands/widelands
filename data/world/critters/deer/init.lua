dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "deer_idle_??.png"),
      hotspot = { 1, 10 },
      fps = 20,
   },
}
add_walking_animations(animations, "walk", dirname, "deer_walk", {15, 25}, 20)

world:new_critter_type{
   name = "deer",
   descname = _ "Deer",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
