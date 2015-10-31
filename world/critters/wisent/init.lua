dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "wisent_idle_??",
      directory = dirname,
      hotspot = { 14, 27 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "wisent_walk", {24, 32}, 20)

world:new_critter_type{
   name = "wisent",
   descname = _ "Wisent",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}