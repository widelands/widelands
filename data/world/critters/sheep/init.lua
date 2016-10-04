dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "sheep_??.png"),
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
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
