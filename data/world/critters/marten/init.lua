dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 11 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "walk", {15, 14}, 20)

world:new_critter_type{
   name = "marten",
   descname = _ "Marten",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
