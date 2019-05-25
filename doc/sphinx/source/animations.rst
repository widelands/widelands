.. _animations:

Animations
==========

This article covers how to get your animations into the Widelands engine.
For information on how to create the animations' images with Blender, see
`GraphicsDevelopment <https://wl.widelands.org/wiki/GraphicsDevelopment/>`_.

Animations are defined as `Lua tables <http://lua-users.org/wiki/TablesTutorial>`_.
All :ref:`map objects <animations_map_object_types>` have a mandatory ``idle`` animation.
They can then have further animations, depending on what their specific capabilities are.

We recommend that you use the :ref:`animations_convenience_functions` below for
the Lua coding, but let's look at an example first to understand which options
are available, and what your image files need to look like:

.. code-block:: lua

   animations = {
      idle = {
         files = path.list_files(path.dirname(__file__) .. "idle_??.png"),
         hotspot = { 5, 7 },
         fps = 4,
         sound_effect = {
            path = "sound/foo/bar",
            priority = 128
         },
         representative_frame = 3,
      },
      working = ...
   }

Let's have a detailed look at the ``idle`` animation:

**idle**
   *Mandatory*. This is the name of the animation. The animation can then be referenced by this name e.g. if you want to trigger it in a production program.

**files**
   *Mandatory*. A template for the image names. Our example will pick up any image from ``idle_00.png`` through ``idle_99.png`` in the specified directory path -- the current path in our example. These images can optionally have corresponding player color mask images called ``idle_00_pc.png`` through ``idle_99_pc.png``. Make sure to include leading 0's in the file names and to have consistent length -- we support 1, 2 and 3 digit numbers in an animation.

**pictures**
   *DEPRECATED*. The same as ``files``.

**hotspot**
   *Mandatory*. Hotspots define a place on a graphic image through its *x* and *y* coordinates that can be used as a handle or reference point to provide control over positioning the image on the map. For example, hotspots help carriers stay on the road, and control the positioning of the wares they carry. Increase ``x`` to shift the animation to the left and ``y`` to shift it upwards.

**fps**
   *Optional*. The frames per second for this animation if you want to deviate from the default fps. This will control the playback speed of the animation. Do not specify this value if you have only 1 animation frame.

**sound_effect**
   *Optional*. Our example will look for the sound files ``bar_00.ogg`` through ``bar_99.ogg`` in the directory ``data/sound/foo`` and play them in sequence. The priority is optional with the default being ``1``, and its range is:

   * **0-127:** Probability between ``0.0`` and ``1.0``, only one instance can be playing at any time
   * **128-254:** Probability between ``0.0`` and ``1.0``, many instances can be playing at any time
   * **255:** Always play


Mipmaps
-------

We support mipmaps for animations. They allow us to provide the same image in different
resolutions for optimum rendering quality. Let's look at an example with a mipmap ``idle`` animation and a normal ``build`` animation:

.. code-block:: lua

   animations = {
      idle = {
         mipmap = {
            {
               scale = 0.5,
               files = path.list_files(dirname .. "idle_0.5_??.png"),
            },
            {
               scale = 1,
               files = path.list_files(dirname .. "idle_1_??.png"),
            },
            {
               scale = 2,
               files = path.list_files(dirname .. "idle_2_??.png"),
            },
            {
               scale = 4,
               files = path.list_files(dirname .. "idle_4_??.png"),
            }
         },
         hotspot = { 5, 7 },
         fps = 4,
         sound_effect = {
            directory = "sound/foo",
            name = "bar",
         },
      },
      build = {
         files = path.list_files(dirname .. "build_??.png"),
         hotspot = { 5, 7 },
      }
   },

The scale of ``1`` is mandatory, and other supported scales are ``0.5``, ``2``
and ``4``.
The base table should no longer contain the ``files`` entry
when you're using a mipmap.
Each mimap entry must define the ``files`` and the ``scale``.
See also :ref:`animations_converting_formats`.

**representative_frame**
   *Optional*. Choose the animation frame that will be displayed in messages and in the encyclopedia as a representative image. Default is 0.

Directional Animations
----------------------

For objects that move around the map, like carriers, ships or animals, there need to be 6 animations for the walking directions northeast ``"ne"``, east ``"e"``, southeast ``"se"``, southwest ``"sw"``, west ``"w"``, and northwest ``"nw"``. So, a "walk" animation would consist of 6 animations called ``"walk_ne"``, ``"walk_e"``, ``"walk_se"``, ``"walk_sw"``, ``"walk_w"``, and ``"walk_nw"``.

Each of these 6 animations will then be defined like the animation above, so we would end up with files called ``walk_ne_00.png``, ``walk_ne_01.png`` ... ``walk_nw_00.png``,  ``walk_nw_01.png`` ..., and for player color: ``walk_ne_00_pc.png``, ``walk_ne_01_pc.png`` ... ``walk_nw_00_pc.png``,  ``walk_nw_01_pc.png``, ...

We also support mipmaps here -- name the files ``walk_ne_0.5_00.png``,
``walk_ne_0.5_01.png`` etc. for scale `0.5`, ``walk_ne_1_00.png``,
``walk_ne_1_01.png`` etc. for scale `1` and so on.



.. _animations_convenience_functions:

Convenience Functions
---------------------

In order to cut down on the manual coding needed, we provide the convenience functions
:any:`add_animation` for static animations and :any:`add_walking_animations` for walking
animations, both of which will also detect mipmaps automatically.
The corresponding ``.lua`` script file is included centrally when the tribe or world
loading is started, so you won't need to include it again. Example:

.. code-block:: lua

   dirname = path.dirname(__file__)

   -- This table will contain the animations
   animations = {}

   -- Add an idle animation with hotspot = {16, 30} and fps = 5
   add_animation(animations, "idle", dirname, "idle", {16, 30}, 5)

   -- Add animations for the 6 directions with hotspot = {16, 30} and fps = 10
   add_walking_animations(animations, "walk", dirname, "walk", {16, 30}, 10)

   -- Add a "walkload" animation. The animation hasn't been created yet in this example, so we reuse the files for the "walk" animation.
   add_walking_animations(animations, "walkload", dirname, "walk", {16, 30}, 10)


   tribes:new_worker_type {
      msgctxt = "fancytribe_worker",
      name = "fancytribe_diligentworker",
      ...

      animations = animations, -- Include the animations table in your map object
      ...
   }

The convenience functions don't support sound effects directly, so you'll have to
add them manually, like this:

.. code-block:: lua

   animations = {}
   add_animation(animations, "work", dirname, "work", {11, 26}, 10)
   animations["work"]["sound_effect"] = {
      name = "bar",
      directory = "sound/foo"
   }


.. _animations_converting_formats:

Converting Animation Formats
----------------------------

When converting a simple file animation to a mipmap animation, follow these steps:

* Use `utils/rename_animation.py` to rename the previous animation, to make sure
  that our version control system will not lose its history, e.g.::

   utils/rename_animation.py data/tribes/workers/fancytribe/diligentworker/walk_ne data/tribes/workers/fancytribe/diligentworker/walk_ne_1
   utils/rename_animation.py data/tribes/workers/fancytribe/diligentworker/walk_nw data/tribes/workers/fancytribe/diligentworker/walk_nw_1
   ...

* Export the new animations from Blender, preferably at all supported scales.
  Only export the higher resolution scales if the textures have sufficient resolution.

* Alternatively, you can use the Java tool MipmapMaker that is contained in the widelands-media repository.
  MipmapMaker accepts high-resolution images as input files (they should be at least 4 times the in-game size)
  and creates correctly named mipmap images for all supported scales for each animation, e.g.:

  java MipmapMaker ~/widelands/data/tribes/workers/fancytribe/diligentworker walk_se 2 true true true 4.0

  MipmapMaker is documented in ``widelands-media/graphics/tools/Graphics Tools (Java)/Readme``.


.. _animations_map_object_types:

Map Object Types
----------------

Each type of map object needs a specific set of animations defined. All map objects have an ``idle`` animation. Specialized map objects have the following additional animations:

Buildings
^^^^^^^^^

For building animations, the hotspot denotes where the road ends at the building.
Typical building animations are:

   **build**
      This is the building's construction animation, which usually consist of 4 frames.

   **idle**
      This animation depicts the building in a non-working or resting state.

   **working**
      The building in a productive working state.

   **unoccupied**
      There is no worker in the building.

   **empty**
      For mines when the mine can't find any more resources.

Any animation other than the ``build`` and ``idle`` animations are referenced in the building's ``programs`` table via the ``animate`` command. For more information on building programs, see :ref:`productionsite_programs`.

For example, the animations for a mine could look like this:

.. code-block:: lua

   dirname = path.dirname(__file__)

   animations = {}
   add_animation(animations, "idle", dirname, "idle", {21, 36})
   add_animation(animations, "build", dirname, "build", {21, 36})
   add_animation(animations, "working", dirname, "working", {21, 36})
   add_animation(animations, "empty", dirname, "empty", {21, 36})


Immovables
^^^^^^^^^^

Unlike for other map objects, the ``idle`` animation needs to be referenced via the ``animate`` command for the default ``program`` in an immovable's ``programs`` table.


Workers
^^^^^^^

Workers can have both non-directional animations and directional animations. The following animations will always be loaded if defined:

   **idle**
      *Mandatory*. This non-directional animation depicts the worker in a non-working or resting state, e.g. a carrier waiting on a road when there are no wares to transport.

   **walk**
      *Mandatory*. A directional animation. The worker is walking towards a destination.

   **walkload**
      *Optional*. A directional animation. The worker is walking while carrying something.

Any further animations like e.g. "plant", "harvest", or "breed" will be referenced in the :ref:`tribes_worker_programs`, under the ``animation`` command.

For example, a fisher's animations could look like this:

.. code-block:: lua

   dirname = path.dirname(__file__)

   animations = {}
   add_animation(animations, "idle", dirname, "idle", {9, 39})
   add_animation(animations, "fishing", dirname, "fishing", {9, 39}, 10)
   add_walking_animations(animations, "walk", dirname, "walk", {10, 38}, 10)
   add_walking_animations(animations, "walkload", dirname, "walk", {10, 38}, 10)

Soldiers
^^^^^^^^

Soldiers have the same animations as workers, plus additional non-directional battle animations. There can be multiple animations for each action in battle to be selected at random.
Each animation for a soldier requires a range of training levels to be specified. An animation will be used only for soldiers within the chosen range. Refer to ``Tribes.new_soldier_type`` for details on the syntax.
For example, attacking towards the west can be defined like this:

.. code-block:: lua

   dirname = path.dirname(__file__)

   animations = {}
   add_animation(animations, "idle", dirname, "idle", {16, 31}, 5)
   add_walking_animations(animations, "walk", dirname, "walk", {16, 31}, 10)
   ...

   add_animation(animations, "atk_ok_w1", dirname, "atk_ok_w1", {36, 40}, 20) -- First attack animation
   add_animation(animations, "atk_ok_w2", dirname, "atk_ok_w2", {36, 40}, 20) -- Second attack animation
   ...

   tribes:new_soldier_type {
      msgctxt = "fancytribe_worker",
      name = "fancytribe_soldier",
      ...

      -- Reference the attack animations in your map object
      attack_success_w = {
         atk_ok_w1 = levels,
         atk_ok_w2 = levels,
      },
      ...
   }

The battle animations are:

   **attack_success_w**
      A successful attack towards the west.

   **attack_success_e**
      A successful attack towards the east.

   **attack_failure_e**
      A failed attack towards the west.

   **attack_failure_w**
      A failed attack towards the west.

   **evade_success_w**
      Successfully evaded an attack from the west.

   **evade_success_e**
      Successfully evaded an attack from the east.

   **evade_failure_e**
      Is being hit by an attack from the west.

   **evade_failure_w**
      Is being hit by an attack from the east.

   **die_w**
      Killed by an attack from the west.

   **die_e**
      Killed by an attack from the east.


Ships
^^^^^

All ships have the following animations:

   **idle**
      The ship is waiting for something to do.

   **sinking**
      The ship is being sunk.

   **sail**
      A directional animation shown while the ship is traveling.



Critters (Animals)
^^^^^^^^^^^^^^^^^^

Critters have an ``idle`` and a ``walk`` animation.
