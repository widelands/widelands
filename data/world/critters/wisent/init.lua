dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "wisent_idle_??.png"),
      hotspot = { 14, 27 },
      fps = 20,
   },
}
add_walking_animations(animations, "walk", dirname, "wisent_walk", {24, 32}, 20)

world:new_critter_type{
   name = "wisent",
   descname = _ "Wisent",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
