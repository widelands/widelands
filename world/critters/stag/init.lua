dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "stag_idle_??",
      directory = dirname,
      hotspot = { 12, 26 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "stag_walk", {25, 30}, 20)

world:new_critter_type{
   name = "stag",
   descname = _ "Stag",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
