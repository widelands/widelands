dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 13 },
      fps = 20,
   },
}

add_walking_animations(animations, "walk", dirname, "walk", {11, 20}, 20)

world:new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
