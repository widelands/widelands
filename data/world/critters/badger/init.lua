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
--
-- .. function:: new_critter_type{table}
--
--    This function adds the definition of a critter to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this critter. It contains the following entries:
--
--    **name**
--        *Mandatory*. A string containing the internal name of this critter, e.g.::
--
--             name = "badger",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--             descname = _("Badger"),
--
--    **animation_directory**
--        *Mandatory*. The location of the animation png files.
--
--    **editor_category**
--        *Deprecated*. The category that is used in the editor tools for placing a critter of this type on the map, e.g.::
--
--             editor_category = "critters_carnivores",
--
--    **size**
--        *Mandatory*. This critter's size in relative units (bigger values mean larger and stronger), within range 1..10, e.g.::
--
--             size = 5,
--
--    **reproduction_rate**
--        *Mandatory*. How likely this critter is to consider reproducing when it finds a mating partner, in %, e.g.::
--
--             reproduction_rate = 10,
--
--    **herbivore**
--        *Optional*. An array of attribute names. If given, this critter considers any immovable with one of these attributes food. E.g.::
--
--             herbivore = { "field" },
--
--    **carnivore**
--        *Optional*. Whether this critter considers all other critter types food. E.g.::
--
--             carnivore = true,
--
--    **appetite**
--        *Mandatory* if ``herbivore`` or ``carnivore`` is given. How likely this critter is to eat when it finds food, in %, e.g.::
--
--             appetite = 20,
--
--    **programs**
--        *Mandatory*. Every critter has an automatic default program, which is to move around the map at random. Additional programs can be defined that other map objects can then call in their programs, e.g.::
--
--             programs = {
--                remove = { "remove" }, -- A hunter will call this after catching this animal
--             },
--
--    **animations**
--        *Optional*. A table containing all file animations for this critter. Every critter
--        needs to have an ``idle`` and a directional ``walk`` animation. Herbivores and carnivores additionally need an ``eating`` animation.
--        Animations can either be defined as file animations in this table or as spritesheet animations
--        as defined in table ``spritesheets``. A mixture of the two animation formats is allowed.
--        See :doc:`animations` for a detailed description of the animation formats.
--
--    **spritesheets**
--        *Optional*. A table containing all spritesheet animations for this critter. Every critter
--        needs to have an ``idle`` and a directional ``walk`` animation. Herbivores and carnivores additionally need an ``eating`` animation.
--        Animations can either be defined as spritesheet animations in this table or as file animations
--        as defined in table ``animations``. A mixture of the two animation formats is allowed.
--        See :doc:`animations` for a detailed description of the animation formats.
--
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("world")
--
--    wl.Descriptions():new_critter_type{
--       name = "badger",
--       descname = _("Badger"),
--       animation_directory = path.dirname(__file__),
--       programs = {
--           remove = { "remove" },
--       },
--       size = 4,
--       reproduction_rate = 70,
--       appetite = 50,
--       carnivore = true,
--
--      spritesheets = {
--         idle = {
--            fps = 20,
--            frames = 7,
--            rows = 4,
--            columns = 2,
--            hotspot = { 9, 12 }
--         },
--         eating = {
--            basename = "idle", -- TODO(Nordfriese): Make animation
--            fps = 20,
--            frames = 7,
--            rows = 4,
--            columns = 2,
--            hotspot = { 9, 12 }
--         },
--         walk = {
--            fps = 20,
--            frames = 20,
--            rows = 5,
--            columns = 4,
--            directional = true,
--            hotspot = { 13, 15 }
--         },
--      },
--    }
--
--    pop_textdomain()

push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "badger",
   descname = _("Badger"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 4,
   reproduction_rate = 70,
   appetite = 50,
   carnivore = true,

   spritesheets = {
      idle = {
         fps = 20,
         frames = 7,
         rows = 4,
         columns = 2,
         hotspot = { 9, 12 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 7,
         rows = 4,
         columns = 2,
         hotspot = { 9, 12 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 13, 15 }
      },
   },

}

pop_textdomain()
