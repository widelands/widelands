dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "marten_idle_??",
      directory = dirname,
      hotspot = { 11, 11 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "marten_walk", {15, 14}, 20)

world:new_critter_type{
   name = "marten",
   descname = _ "Marten",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}