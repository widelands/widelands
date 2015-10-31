dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "bunny_idle",
      directory = dirname,
      hotspot = { 4, 9 },
   },
}
add_walking_animations(animations, dirname, "bunny_walk", {5, 9}, 4)

world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}