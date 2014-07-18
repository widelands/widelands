dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_directory(dirname, "wildboar_idle_nw_\\d+.png"),
      hotspot = { 10, 18 },
      fps = 20,
   },
}
add_walking_animations(animations, dirname, "wildboar_walk", {20, 22}, 20)

world:new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}



--attrib=eatable
--program=remove

--[remove]
--0=remove

--[idle]
--pics=wildboar_idle_nw_??.png
--hotspot=10 18
--fps=20

--[walk]
--dirpics=wildboar_walk_!!_??.png
--hotspot=20 22
--fps=20
