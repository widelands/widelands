dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "fallentree_idle.png" },
      player_color_masks = {},
      hotspot = { 2, 31 },
   },
}

world:new_immovable_type{
   name = "fallentree",
   descname = _"Fallen Tree",
   size = "none",
   attributes = {},
   programs = {},
   animations = animations,
}