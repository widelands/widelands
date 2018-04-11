-- RST
-- mapobjects.lua
-- --------------
--
-- This script contains some convenience functions for loading the tribes
-- and world.
--

-- RST
-- .. function:: print_loading_message(preamble, func)
--
-- Prints a message containing the duration func() required to run.
--
--    :arg preamble: The name of the item that was being loaded
--    :arg func: The function to time execution off.
function print_loading_message(preamble, func)
   local start = ticks()
   func()
   print(("%s: %dms"):format(preamble, ticks() - start))
end


-- RST
-- .. function:: add_walking_animations(animationtable, animationname, dirname, basename, hotspot[, fps])
--
--    Adds 6 walk or sail animations - one for each walking direction - to 'animationtable'.
--
--    :arg animationtable: A table that the animation data is added to. It may already contain some animations.
--    :arg animationname: The name of the animation to be added, e.g. ``walkload``.
--    :arg dirname: The name of the directory where the animation image files are located.
--    :arg basename: The basename of the animation files. The filenames of the animation files need to have the format ``<basename>_(e|ne|se|sw|w|nw)_\d+.png``
--    :arg hotspot: The hotspot coordinates for blitting, e.g. ``{ 2, 20 }``.
--    :arg fps: Frames per second. Only use this if the animation has more than 1 frame, and if you need to deviate from the default frame rate.
function add_walking_animations(animationtable, animationname, dirname, basename, hotspot, fps)
   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      animationtable[animationname .. "_" .. dir] = {
         files = path.list_files(dirname .. basename .. "_" .. dir ..  "_??.png"),
         hotspot = hotspot,
      }
      if (fps ~= nil) then
         animationtable[animationname .. "_" .. dir]["fps"] = fps
      end
   end
end

-- RST
-- .. function:: add_directional_animations(animationtable, animationname, dirname, basename, hotspot, scales[, fps])
--
--    Adds 6 walk or sail animations - one for each walking direction - to 'animationtable',
--    using a mimpap for multiple scales.
--
--    :arg animationtable: A table that the animation data is added to. It may already contain some animations.
--    :arg animationname: The name of the animation to be added, e.g. ``walkload``.
--    :arg dirname: The name of the directory where the animation image files are located.
--    :arg basename: The basename of the animation files. The filenames of the animation files need to have the format ``<basename>_(e|ne|se|sw|w|nw)_\d+.png``
--    :arg hotspot: The hotspot coordinates for blitting, e.g. ``{ 2, 20 }``.
--    :arg scales: The mipmap scales to be used, e.g. ``{ 0.5, 1, 2, 4 }``.
--    :arg fps: Frames per second. Only use this if the animation has more than 1 frame, and if you need to deviate from the default frame rate.
function add_directional_animations(animationtable, animationname, dirname, basename, hotspot, scales, fps)

   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      mipmap = {}
      for scale_idx, current_scale in ipairs(scales) do
         table.insert(
            mipmap,
            {
               scale = current_scale,
               files = path.list_files(dirname .. basename .. "_" .. dir ..  "_" .. current_scale .. "_??.png"),
            }
         )
      end
      animationtable[animationname .. "_" .. dir] = {
         mipmap = mipmap,
         hotspot = hotspot,
      }
      if (fps ~= nil) then
         animationtable[animationname .. "_" .. dir]["fps"] = fps
      end
   end
end

