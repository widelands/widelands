.. _toc_lua_world:

Scripts for World
=================

All world elements for Widelands are defined in ``data/world``. There are 2 main
scripts:

* ``data/world/init.lua``: This file lists all scripts that define the world
  elements such as critters, immovables (including trees), resources and
  terrains. It bootstraps the world used in our maps and also defines the editor
  categories.
* ``data/world/map_generation.lua``: This file is used by the map generator.
  You can ignore it while designing your world, but you can add new elements to
  be included in automatically generated maps if you want.

Critters, immovables, resources and terrains are defined in their respective
``init.lua`` and ``register.lua`` script files.

.. toctree::
   :maxdepth: 3

   autogen_toc_lua_world_defining
   autogen_toc_lua_world_units
   autogen_toc_lua_world_other
