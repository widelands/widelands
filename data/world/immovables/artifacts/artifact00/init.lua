-- RST
-- .. _lua_world_immovables:
--
-- Immovables
-- ----------
--
-- Immovables are entities owned by the world that are placed in a fixed position
-- on the map. They include trees, rocks, artifacts, and eye candy.
--
-- Immovables are defined in
-- ``data/world/immovables/<immovable_name>/init.lua``.
-- Some immovables are grouped into subdirectories, e.g. trees and rocks.
-- There are also some specialized immovables:
--
-- * `Artifacts`_
-- * `Rocks`_
-- * `Trees`_
--
-- .. function:: new_immovable_type{table}
--
--    This function adds the definition of an immovable to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--       to this immovable. It contains the following entries:
--
--    **name**
--        *Mandatory*. A string containing the internal name of this immovable, e.g.::
--
--            name = "alder_summer_old",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--            descname = _"Alder (Old)",
--
--    **species**
--        *Mandatory for trees*. `Trees`_ have 4 variants (sapling, pole, mature, and old),
--        so we will want a simplified translatable display name for the editor help, e.g.::
--
--            species = _"Alder",
--
--    **editor_category**
--        *Deprecated*. The category that is used in the editor tools for placing an
--        immovable of this type on the map, e.g.::
--
--            editor_category = "trees_deciduous",
--
--    **size**
--        *Optional*. The size of the immovable. Defaults to ``none`` -
--        note that immovables with size ``none`` will be removed when
--        their space is needed for a road. Possible values
--        are ``none``, ``small``, ``medium``, and ``big``, e.g.::
--
--            size = "small",
--
--    **programs**
--        *Mandatory*. Every immovable has an automatic default program,
--        which will simply play the ``idle`` animation. Leave the table empty if
--        this is all you need. You can also overwrite this default ``program``,
--        like this::
--
--            program = {
--               "animate=idle duration:1m20s500ms",
--               "remove=chance:2.35%",
--               "grow=alder_summer_old",
--            },
--
--       See :ref:`immovable_programs`.
--
--    **terrain_affinity**
--        *Mandatory for trees*. If your immovable is a tree (c.f. `Trees`_), you will need to specify its
--        terrain affinity, which will determine its chances to survive on different
--        terrains. Terrain affinity is a table and looks like this::
--
--          terrain_affinity = {
--             -- Temperature is in arbitrary units.
--             preferred_temperature = 125,
--
--             -- The preferred humibity. This is a value between 0 and 1000, with 1000 being very wet.
--             preferred_humidity = 650,
--
--             -- The preferred fertility. This is a value between 0 and 1000, with 1000 being very fertile.
--             preferred_fertility = 600,
--
--             -- A value in [0, 99] that defines how well this immovable can deal with non-ideal terrain.
--             -- A lower value means that it is less picky, i.e. it can deal better with it.
--             pickiness = 60,
--          }
--
--    **animations**
--        *Optional*. A table containing all file animations for this immovable.
--        Every immovable needs to have at least an ``idle`` animation.
--        Animations can either be defined as file animations in this table or as spritesheet animations
--        as defined in table ``spritesheets``. A mixture of the two animation formats is allowed.
--        See :doc:`animations` for a detailed description of the animation format.
--
--    **spritesheets**
--        *Optional*. A table containing all spritesheet animations for this immovable.
--        Every immovable needs to have at least an ``idle`` animation.
--        Animations can either be defined as spritesheet animations in this table or as file animations
--        as defined in table ``animations``. A mixture of the two animation formats is allowed.
--        See :doc:`animations` for a detailed description of the animation format.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("world")
--
--    dirname = path.dirname(__file__)
--
--    wl.Descriptions():new_immovable_type{
--       name = "artifact00",
--       descname = _ "Artifact",
--       animation_directory = dirname
--       size = "small",
--       programs = {},
--       animations = {
--          idle = {
--             hotspot = { 14, 20 },
--          },
--       }
--    }
--
--    pop_textdomain()
--
-- Artifacts
-- ^^^^^^^^^
-- Artifacts are immovables that players can hunt for when using the "Artifacts" win condition.
-- They need to define the ``artifact`` attribute in order for the win condition to find them,
-- and should be placed in the editor_category "artifacts".
--
-- Rocks
-- ^^^^^
-- Rocks are a special type of immovable that can be quarried by a stonecutter.
-- They need to define the ``rocks`` attribute in order for the stonecutter to find them,
-- and should be placed in the editor_category "rocks".
-- Rocks will shrink over time and disappear eventually while being quarried.
-- This shrinking is implemented via their ``shrink`` program, which will be triggered
-- by the stonecutter. Rocks usually come in series of 6 immovables:
--
-- * **<rock_name>6 - <rock_name>2**: These rocks will transform into the next smaller rock by calling ``transform``
--   in their ``shrink`` program.
--
-- * **<rock_name>1**: This is the smallest rock, so it needs to call ``remove`` in its ``shrink``
--   program in order to disappear from the map.
--
-- Trees
-- ^^^^^
--
-- Trees are a special type of immovable, and should be placed in the editor_category
-- "trees_coniferous", "trees_deciduous", "trees_palm", or "trees_wasteland".
-- Because they will grow with time, this growth is represented by transforming one
-- tree into the next through the ``grow`` command in their program.
-- They also all need to define a species name and their terrain affinity (see above).
-- Trees will grow in 4 stages:
--
-- * **Sapling**: A sapling will be seeded by another tree or planted by a forester.
--   All saplings have the attribute ``"tree_sapling"``.
-- * **Pole**: A young, slender tree that is not able to seed other trees yet.
--   A pole has no special attributes.
-- * **Mature**: A visually grown tree but without any special attributes, just like the pole.
-- * **Old**: Old trees can be felled, which is defined by the attribute ``tree``.
--   They will also die off and seed a new tree via the ``transform`` and ``seed``
--   commands in their program. They also need to specify a ``fall`` program,
--   which will be triggered when a lumberjack fells them.

push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "artifact00",
   descname = _ "Artifact",
   size = "small",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 14, 20 },
      },
   }
}

pop_textdomain()
