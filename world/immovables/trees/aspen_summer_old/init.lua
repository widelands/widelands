dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_summer_old",
   descname = _ "Aspen (Old)",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 15",
         "seed=tree1_t 70",
      },
      fall = {
         "animate=falling 1200",
         "transform=fallentree",
      },
   },
   terrain_affinity = {
      -- NOCOM(#sirver): this should probably be expressed in attributes, not in terrain names.
      steppe = 100,
      berg1 = 7,
      strand = 3,
      wiese1 = 100,
      wiese2 = 200,
      wiese3 = 200,
      berg2 = 7,
      berg3 = 7,
      berg4 = 7,
      wiese4 = 255,
      bergwiese = 50,
   },
   animations = {
      idle = {
         pictures = path.glob(dirname, "idle_*.png"),
         player_color_masks = {},
         hotspot = { 23, 58 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird1.ogg",
         },
      },
      falling = {
         pictures = path.glob(dirname, "f_tree1_*.png"),
         player_color_masks = {},
         hotspot = { 20, 59 },
         fps = 10,
      },
   },
}
