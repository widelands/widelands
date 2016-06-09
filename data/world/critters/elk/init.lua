dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "elk_idle_??.png"),
      hotspot = { 15, 27 },
      fps = 20,
      sound_effect = {
         directory = "sound/animals",
         name = "elk",
      },
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
