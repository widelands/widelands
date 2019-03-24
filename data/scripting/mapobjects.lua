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
-- .. function:: add_walking_animations(table, animationname, dirname, basename, hotspot, fps)
--
--    Adds 6 walk or sail animations - one for each walking direction - to 'table'.
--
--    :arg table: A table that the animation data is added to. It may already contain some animations.
--    :arg animationname: The name of the animation to be added, e.g. ``walkload``.
--    :arg dirname: The name of the directory where the animation image files are located.
--    :arg basename: The basename of the animation files. The filenames of the animation files need to have the format ``<basename>_(e|ne|se|sw|w|nw)_\d+.png``
--    :arg hotspot: The hotspot coordinates for blitting, e.g. ``{ 2, 20 }``.
--    :arg fps: Frames per second. Only use this if the animation has more than 1 frame, and if you need to deviate from the default frame rate.
function add_walking_animations(table, animationname, dirname, basename, hotspot, fps)
   if (fps ~= nil) then
      for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
         table[animationname .. "_" .. dir] = {
            pictures = path.list_files(dirname .. basename .. "_" .. dir ..  "_??.png"),
            hotspot = hotspot,
            fps = fps,
         }
      end
   else
      for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
         table[animationname .. "_" .. dir] = {
            pictures = path.list_files(dirname .. basename .. "_" .. dir ..  "_??.png"),
            hotspot = hotspot,
         }
      end
   end
end
