.. _animations:

Animations
==========

Animations are defined as Lua tables. All map objects have a mandatory ``idle`` animation. They can then have further animations, depending on what their specific capabilities are. Let's look at an example::

   animations = {
      idle = {
         pictures = path.list_files(path.dirname(__file__) .. "idle_??.png"),
         hotspot = { 5, 7 },
         scale = 2.5,
         fps = 4,
         sound_effect = {
            directory = "sound/foo",
            name = "bar",
         },
      },
      working = ...
   }

Let's have a detailed look at the ``idle`` animation:

**idle**
   *Mandatory*. This is the name of the animation. The animation can then be referenced by this name e.g. if you want to trigger it in a production program.

**pictures**
   *Mandatory*. A template for the image names. Our example will pick up any image from ``idle_00.png`` through ``idle_99.png`` in the specified directory path – the current path in our example. These images can optionally have corresponding player color mask images called ``idle_00_pc.png`` through ``idle_99_pc.png``. Make sure to include leading 0's in the file names.

**hotspot**
   *Mandatory*. Hotspots define a place on a graphic image through its *x* and *y* coordinates that can be used as a handle or reference point to provide control over positioning the image on the map. For example, hotspots help carriers stay on the road, and control the positioning of the wares they carry. Increase ``x`` to shift the animation to the left and ``y`` to shift it upwards.

**fps**
   *Optional*. The frames per second for this animation if you want to deviate from the default fps. This will control the playback speed of the animation. Do not specify this value if you have only 1 animation frame.

**scale**
   **DEPRECATED**. If the animation should be blitted at any other scale than 1:1,
   specify the float value here. For example, if the animation images are 2.5 times the size of what should be blitted at default zoom, use ``scale = 2.5``.

**sound_effect**
   *Optional*. Our example will look for the sound files ``bar_00.ogg`` through ``bar_99.ogg`` in the directory ``data/sound/foo`` and play them in sequence.


Directional Animations
----------------------

For objects that move around the map, like carriers, ships or animals, there need to be 6 animations for the walking directions northeast ``"ne"``, east ``"e"``, southeast ``"se"``, southwest ``"sw"``, west ``"w"``, and northwest ``"nw"``. So, a "walk" animation would consist of 6 animations called ``"walk_ne"``, ``"walk_e"``, ``"walk_se"``, ``"walk_sw"``, ``"walk_w"``, and ``"walk_nw"``.

Each of these 6 animations will then be defined like the animation above, so we would end up with pictures called ``walk_ne_00.png``, ``walk_ne_01.png`` ... ``walk_nw_00.png``,  ``walk_nw_01.png`` ..., and for player color: ``walk_ne_00_pc.png``, ``walk_ne_01_pc.png`` ... ``walk_nw_00_pc.png``,  ``walk_nw_01_pc.png``, ...

In order to cut down on the manual coding needed, we have a convenience function :any:`add_walking_animations` to define the animation tables for walking animations. The corresponding ``.lua`` script file is included centrally when the tribe or world loading is started, so you won't need to include it again.

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

For example, a mine could look like this::

   dirname = path.dirname(__file__)

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 21, 36 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 21, 36 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 21, 36 },
      },
      empty = {
         pictures = path.list_files(dirname .. "empty_??.png"),
         hotspot = { 21, 36 },
      },
   },


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

For example, a fisher could look like this::

   dirname = path.dirname(__file__)

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 9, 39 },
      },
      fishing = {
         pictures = path.list_files(dirname .. "fishing_??.png"),
         hotspot = { 9, 39 },
         fps = 10
      }
   }
   add_walking_animations(animations, "walk", dirname, "walk", {10, 38}, 10)
   add_walking_animations(animations, "walkload", dirname, "walk", {10, 38}, 10)

Soldiers
^^^^^^^^

Soldiers have the same animations as workers, plus additional non-directional battle animations. There can be multiple animations for each action in battle to be selected at random.
For example, attacking towards the west can be defined like this::

   dirname = path.dirname(__file__)

   animations = {
      ...
      atk_ok_w1 = {
         pictures = path.list_files(dirname .. "atk_ok_w1_??.png"),
         hotspot = { 36, 40 },
         fps = 20
      },
      atk_ok_w2 = {
         pictures = path.list_files(dirname .. "atk_ok_w2_??.png"),
         hotspot = { 36, 40 },
         fps = 20
      },
      ...
   },

   attack_success_w = {
      "atk_ok_w1",
      "atk_ok_w2"
   },

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
