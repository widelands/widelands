dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         path = "sound/farm/sheep",
      },
      hotspot = { 8, 16 },
      fps = 20,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {15, 25}, 20)

world:new_critter_type{
   name = "sheep",
   descname = _ "Sheep",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
