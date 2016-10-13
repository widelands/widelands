dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "bunny_idle.png"),
      hotspot = { 4, 9 },
   },
}
add_walking_animations(animations, "walk", dirname, "bunny_walk", {5, 9}, 4)

world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
