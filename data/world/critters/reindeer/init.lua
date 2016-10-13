dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "reindeer_idle_??.png"),
      hotspot = { 23, 21 },
      fps = 20,
   },
}
add_walking_animations(animations, "walk", dirname, "reindeer_walk", {25, 30}, 20)

world:new_critter_type{
   name = "reindeer",
   descname = _ "Reindeer",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
