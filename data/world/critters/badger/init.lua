-- RST
-- .. _lua_world_critters:
--
-- Critters (Animals)
-- ------------------
--
-- Critters are entities owned by the world that will move around the map at random, usually animals. On how to add fish to the map, see :ref:`lua_world_resources`.
--
-- Critters are defined in
-- ``data/world/critters/<critter_name>/init.lua``.

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 9, 12 },
      fps = 20,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {13, 15}, 20)

-- RST
-- .. function:: new_critter_type{table}
--
--    This function adds the definition of a critter to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this critter. It contains the following entries:
--
--    **name**
--        *Mandatory*. A string containing the internal name of this critter, e.g.::
--
--            name = "badger",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--            descname = _"Badger",
--
--    **editor_category**
--        *Mandatory*. The category that is used in the editor tools for placing a critter of this type on the map, e.g.::
--
--            editor_category = "critters_carnivores",
--
--    **attributes**
--        *Mandatory*. Attributes can be used by other programs to identify a class of critters, e.g.::
--
--            attributes = { "eatable" }, -- This animal can be hunted
--
--    **programs**
--        *Mandatory*. Every critter has an automatic default program, which is to move around the map at random. Additional programs can be defined that other map objects can then call in their programs, e.g.::
--
--            programs = {
--               remove = { "remove" }, -- A hunter will call this after catching this animal
--            },
--
--    **animations**
--        *Mandatory*. A table containing all animations for this critter. Every critter
--        needs to have an ``idle`` and a directional ``walk`` animation.
--        See :doc:`animations` for a detailed description of the animation format.
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
