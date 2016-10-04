dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 4, 9 },
   },
}
add_walking_animations(animations, dirname, "walk", {5, 9}, 4)

world:new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
