dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 14, 27 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "walk", {24, 32}, 20)

world:new_critter_type{
   name = "wisent",
   descname = _ "Wisent",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
