dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "deer_idle_??",
      directory = dirname,
      hotspot = { 1, 10 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "deer_walk", {15, 25}, 20)

world:new_critter_type{
   name = "deer",
   descname = _ "Deer",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}