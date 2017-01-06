Animations
==========

Animations are defined as Lua tables. They look like this::

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

idle
   *Mandatory*. This is the name of the animation. It can be referenced by this name e.g. if you want to call it in a production program.

pictures
   *Mandatory*. A template for the image names. Our example will pick up any image from ``idle_00.png`` through ``idle_99.png`` in the specified directory path – the current path in our example. These images can optionally have corresponding player color mask images called ``idle_00_pc.png`` through ``idle_99_pc.png``.

hotspot
   *Mandatory*. The x, y coordinates for adjusting the placement of the animation. Increase ``x`` to shift the animation to the left and ``y`` to shift it upwards.

fps
   *Optional*. The frames per second for this animation if you want to deviate from the default fps. Do not specify this value if you have only 1 animation frame.

scale
   *Optional*. If the animation should be blitted at any other scale than 1:1,
   specify the float value here. For example, if the animation images are 2.5 times the size of what should be blitted at default zoom, use ``scale = 2.5``.

sound_effect
   *Optional*. Our example will look for the sound files ``bar_00.ogg`` through ``bar_99.ogg`` in the directory ``data/sound/foo`` and play them in sequence.
