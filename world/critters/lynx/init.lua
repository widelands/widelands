dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.glob(dirname, "lynx_idle_\\d+.png"),
      player_color_masks = {},
      hotspot = { 8, 14 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "lynx_walk", {11, 21}, 20)

world:new_critter_type{
   name = "lynx",
   descname = _ "Lynx",
   swimming = false,
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
