dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "elk_idle_??",
      directory = dirname,
      hotspot = { 15, 27 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "elk_walk", {21, 34}, 20)

world:new_critter_type{
   name = "elk",
   descname = _ "Elk",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
