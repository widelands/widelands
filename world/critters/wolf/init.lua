dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "wolf_idle_??.png"),
      hotspot = { 8, 15 },
      fps = 10,
   },
}
add_walking_animations(animations, dirname, "wolf_walk", {19, 19}, 20)

world:new_critter_type{
   name = "wolf",
   descname = _ "Wolf",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
