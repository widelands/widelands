dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "chamois_idle_??",
      directory = dirname,
      hotspot = { 11, 13 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "chamois_walk", {11, 20}, 20)

world:new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
