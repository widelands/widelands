dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "brownbear_idle_??.png"),
      hotspot = {21, 16},
      fps = 20
   },
}
add_walking_animations(animations, "walk", dirname, "brownbear_walk", {24, 24}, 4)

world:new_critter_type{
   name = "brownbear",
   descname = _ "Brown bear",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
