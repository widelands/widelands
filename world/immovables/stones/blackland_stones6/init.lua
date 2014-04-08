dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "stones6",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      program = {
         "animate=idle",
         "transform=stones5"
      }
   },
   animations = animations
}

