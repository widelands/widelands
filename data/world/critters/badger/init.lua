dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 12 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "walk", {13, 15}, 20)

world:new_critter_type{
   name = "badger",
   descname = _ "Badger",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
