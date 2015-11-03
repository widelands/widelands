dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "sheep_??",
      directory = dirname,
      sound_effect = {
         directory = "sound/farm",
         name = "sheep",
      },
      hotspot = { 8, 16 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "sheep_walk", {15, 25}, 20)

world:new_critter_type{
   name = "sheep",
   descname = _ "Sheep",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
