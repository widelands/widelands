dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.glob(dirname, "fox_idle_\\d+.png"),
      player_color_masks = {},
      hotspot = { 10, 13 },
      fps = 10,
   },
}
add_walking_animations(animations, dirname, "fox_walk", {11, 14}, 20)

world:new_critter_type{
   name = "fox",
   descname = _ "Fox",
   swimming = false,
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
