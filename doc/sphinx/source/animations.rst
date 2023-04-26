.. _animations:

Animations
==========

This article covers how to get your animations into the Widelands engine.
For information on how to create the animations' images with Blender, see
`GraphicsDevelopment <https://www.widelands.org/wiki/GraphicsDevelopment/>`_.

The Blender export will give you a list of files for each animation.
We call this a "file" animation.
Once you have tested your animation in Widelands and are satisfied with the results,
we recommend that you convert it to a :ref:`spritesheet <animations_spritesheets>`.
This will save disk space and make animation loading faster.

Animations are defined as `Lua tables <http://lua-users.org/wiki/TablesTutorial>`_.
All :ref:`map objects <animations_map_object_types>` have a mandatory ``idle`` animation.
They can then have further animations, depending on what their specific capabilities are.


File Animations
---------------

First, let's look at an example to understand which options are available,
and what your image files need to look like:

.. code-block:: lua

   -- Tell the engine where the png files are located
   animation_directory = path.dirname(__file__),

   -- Define the animations
   animations = {
      idle = {
         hotspot = { 5, 7 },
         fps = 4,
         sound_effect = {
            path = "sound/foo/bar",
            priority = "5.4%",
            allow_multiple = true,
         },
         representative_frame = 3,
      },
      walkload = {
         basename = "walk",
         hotspot = { 5, 7 },
         fps = 4,
         directional = true
      },
      ...
   }

Let's have a detailed look at the ``idle`` animation:

**idle**
   *Mandatory*. This is the name of the animation. The animation can then be referenced by this name e.g. if you want to trigger it in a production program.

**directory**
   *DEPRECATED*. The directory where the animation image files are located. Replaced by *animation_directory* in the Unit's definition.

**basename**
   *DEPRECATED*. The filename prefix for the image files. The plan is to make the file name prefixes identical to the animation name and then to get rid of this parameter.

   Our example will pick up any image from ``idle_00.png`` through ``idle_99.png`` in the specified directory path -- the current path in our example. These images can optionally have corresponding player color mask images called ``idle_00_pc.png`` through ``idle_99_pc.png``. Make sure to include leading 0's in the file names and to have consistent length -- we support 1, 2 and 3 digit numbers in an animation.
   If your animation contains only one file, you can also call it ``idle.png`` (and ``idle_pc.png`` for the player color mask) without ``_`` or any numbers in the file name.
   You can omit the basename if it's identical to the animation's name.

   We support *mipmaps* for animations. They allow us to provide the same image in different resolutions for optimum rendering quality.
   For using mipmaps, simply name your files accordingly, and the engine will pick them up. e.g. ``idle_0.5_00.png`` will be rendered at scale ``0.5``, and ``idle_1_00.png`` will be rendered at the neutral scale ``1``.
   The scale of ``1`` is mandatory, and other supported scales are ``0.5``, ``2`` and ``4``.

**pictures**
   *DEPRECATED*. This is older code that is slowly being phased out - do not use this parameter.
   A table with full directory and file names.

**directional**
   *Optional*. If this is ``true``, indicates to the engine that this is a directional animation. In our example, the engine will create a set of 6 animations called ``"walk_ne"``, ``"walk_e"``, ``"walk_se"``, ``"walk_sw"``, ``"walk_w"``, and ``"walk_nw"``. See :ref:`animations_directional` below.

**hotspot**
   *Mandatory*. Hotspots define a place on a graphic image through its *x* and *y* coordinates that can be used as a handle or reference point to provide control over positioning the image on the map. For example, hotspots help carriers stay on the road, and control the positioning of the wares they carry. Increase ``x`` to shift the animation to the left and ``y`` to shift it upwards.

**fps**
   *Optional*. The frames per second for this animation if you want to deviate from the default fps. This will control the playback speed of the animation. Do not specify this value if you have only 1 animation frame.

**sound_effect**
   *Optional*. Our example will look for the sound files ``bar_00.ogg`` through ``bar_99.ogg`` in the directory ``data/sound/foo`` and play one of them at random. The priority ranges from ``0.01%`` to ``100%`,` and it is the percent chance of the sound being played. If ``allow_multiple = true``, the same sound can be played by multiple map objects at the same time.


.. _animations_directional:

Directional Animations
----------------------

For objects that move around the map, like carriers, ships or animals, there need to be 6 animations for the walking directions northeast ``"ne"``, east ``"e"``, southeast ``"se"``, southwest ``"sw"``, west ``"w"``, and northwest ``"nw"``. So, a "walk" animation would consist of 6 animations called ``"walk_ne"``, ``"walk_e"``, ``"walk_se"``, ``"walk_sw"``, ``"walk_w"``, and ``"walk_nw"``.

Each of these 6 animations will then be defined like the animation above, so we would end up with files called ``walk_ne_00.png``, ``walk_ne_01.png`` ... ``walk_nw_00.png``,  ``walk_nw_01.png`` ..., and for player color: ``walk_ne_00_pc.png``, ``walk_ne_01_pc.png`` ... ``walk_nw_00_pc.png``,  ``walk_nw_01_pc.png``, ...

We also support mipmaps here -- name the files ``walk_ne_0.5_00.png``,
``walk_ne_0.5_01.png`` etc. for scale `0.5`, ``walk_ne_1_00.png``,
``walk_ne_1_01.png`` etc. for scale `1` and so on.

In the Lua code, the option ``directional = true`` will signal to the engine that it needs to treat it as a directional animation.

.. _animations_spritesheets:

Spritesheet Animations
----------------------

The same map object can have a mix of file and spritesheet animations.
For converting a file animation to a spritesheet animation, use the command line
to call ``./wl_create_spritesheet`` from the Widelands program directory.
This will print the command line options to use for converting your animation.

After conversion, you will need to delete the old files and copy over the new files,
then delete the old animation code in the map object's ``init.lua`` file and add the new code.
The tool will print the new code to the console for you, so all you need to do is copy/paste
and then add any missing optional parameters back in.

Note that the table for spritesheets is called ``spritesheets``, not ``animations``!
This distinction is necessary for performance reasons.

Spritesheets have three additional mandatory parameters so that the engine can
identify the individual textures in the sheet:

**frames**
  The number of frames (images) that this animation has. Equal to the number of
  files in a file animation.

**columns**
  The number of image columns in the spritesheet.

**rows**
  The number of image rows in the spritesheet.


Here's the example from above as spritesheets:

.. code-block:: lua

   spritesheets = {
      idle = {
         fps = 4,
         frames = 150,
         rows = 13,
         columns = 12,
         hotspot = { 5, 7 }
         sound_effect = {
            path = "sound/foo/bar",
            priority = "5.4%",
            allow_multiple = true
         },
         representative_frame = 3,
      },
      walk = {
         fps = 4,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 5, 7 }
      },
      ...
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

  ``java MipmapMaker ~/widelands/data/tribes/workers/fancytribe/diligentworker walk_se 2 true true true 4.0``

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

   animation_directory = path.dirname(__file__),
   animations = {
      idle = {
         hotspot = { 21, 36 },
      },
      build = {
         hotspot = { 21, 36 },
      },
      working = {
         hotspot = { 21, 36 },
      },
      empty = {
         hotspot = { 21, 36 },
      },
   }


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

   animation_directory = path.dirname(__file__),
   -- A file animation for the idle. Ideally, we'll only have those if the animation only has 1 frame.
   animations = {
      idle = {
         hotspot = { 7, 33 },
      },
   },
   spritesheets = {
      -- A directional animation as a spritesheet
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 8, 32 }
      },
      -- A stationary animation as a spritesheet
      fish = {
         fps = 10,
         frames = 30,
         rows = 6,
         columns = 5,
         hotspot = { 7, 33 }
      }
   }

Soldiers
^^^^^^^^

Soldiers have the same animations as workers, plus additional non-directional battle animations. There can be multiple animations for each action in battle to be selected at random.
Each animation for a soldier requires a range of training levels to be specified. An animation will be used only for soldiers within the chosen range. Refer to ``Tribes.new_soldier_type`` for details on the syntax.
For example, attacking towards the west can be defined like this:

.. code-block:: lua

   level_rookie = {
      min_health = 0,
      min_attack = 0,
      min_defense = 0,
      min_evade = 0,
      max_health = 0,
      max_attack = 3,
      max_defense = 2,
      max_evade = 0,
   }

   wl.Descriptions():new_soldier_type {
      name = "fancytribe_soldier",
      ...

      animation_directory = path.dirname(__file__),

      animations = {
         idle = {
            hotspot = { 16, 31 },
            fps = 5
         },
         -- First attack animation
         atk_ok_w1 = {
            hotspot = { 30, 36 },
            fps = 10
         },
         -- Second attack animation
         atk_ok_w2 = {
            hotspot = { 30, 36 },
            fps = 10
         },
         ...
         walk = {
            hotspot = { 16, 31 },
            fps = 10,
            directional = true
         },
      },

      -- Reference the attack animations in your map object
      attack_success_w = {
         atk_ok_w1 = level_rookie,
         atk_ok_w2 = level_rookie,
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
