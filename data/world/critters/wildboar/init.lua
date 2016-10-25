dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "wildboar_idle_??.png"),
      hotspot = { 10, 18 },
      fps = 20,
      sound_effect = {
         directory = "sound/animals",
         name = "boar",
      },
   },
}
add_walking_animations(animations, dirname, "wildboar_walk", {20, 22}, 20)

world:new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
