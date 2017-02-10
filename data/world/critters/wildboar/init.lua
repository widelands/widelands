dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 18 },
      fps = 20,
      sound_effect = {
         directory = "sound/animals",
         name = "boar",
      },
   },
}

add_walking_animations(animations, "walk", dirname, "walk", {20, 22}, 20)

world:new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
