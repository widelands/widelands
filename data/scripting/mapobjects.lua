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
--    Prints a message containing the duration func() required to run.
--
--    :arg preamble: The name of the item that was being loaded
--    :type preamble: :class:`string`
--    :arg func: The function to time execution off.
--    :type func: :class:`function`
function print_loading_message(preamble, func)
   local start = ticks()
   func()
   print(("%s: %dms"):format(preamble, ticks() - start))
end


-- Helper function for finding animation files with varying numbering length
function get_animation_files(prefix)
   local animation_files = path.list_files(prefix .. "_??.png")
   if #animation_files < 1 then
      animation_files = path.list_files(prefix .. "_???.png")
   end
   if #animation_files < 1 then
      animation_files = path.list_files(prefix .. "_?.png")
   end
   return animation_files
end


-- The mipmap scales supported by the engine.
-- Ensure that this always matches kSupportedScales in animations.cc
local supported_scales = { 0.5, 1, 2, 4 }


-- RST
-- .. function:: add_animation(animationtable, animationname, dirname, basename, hotspot [, fps])
--
--    Convenience function for adding an animation to `animationtable`.
--    Supports both simple animations and mipmaps.
--    See :ref:`animations` for more documentation and code examples.
--
--    :arg animationtable: The table that the animation data will be added to.
--       It may already contain some animations.
--    :type animationtable: :class:`table`
--    :arg animationname: The name of the animation to be added, e.g. ``idle``.
--    :type animationname: :class:`string`
--    :arg dirname: The name of the directory where the animation image files are located.
--    :type dirname: :class:`string`
--    :arg basename: The basename of the animation files. The filenames of the
--       animation files need to have the format ``<basename>_\d{2,3}.png`` for simple
--       file animations, and  ``<basename>_<scale>_\d{2,3}.png`` for mipmaps.
--       Supported scales are ``0.5``, ``1``, ``2`` and ``4``.
--    :type basename: :class:`string`
--    :arg hotspot: The hotspot coordinates for blitting, e.g. ``{2, 20}``.
--    :type hotspot: :class:`array`
--    :arg fps: Frames per second. Only use this if the animation has more than
--       1 frame, and if you need to deviate from the default frame rate.
--    :type fps: :class:`integer`
function add_animation(animationtable, animationname, dirname, basename, hotspot, fps)
   mipmap = {}
   for scale_idx, current_scale in ipairs(supported_scales) do
      local listed_files = get_animation_files(dirname .. basename .. "_" .. current_scale)
      if #listed_files > 0 then
         table.insert(
            mipmap,
            {
               scale = current_scale,
               files = listed_files,
            }
         )
      end
   end
   if #mipmap < 1 then
      table.insert(
      mipmap,
         {
            scale = 1,
            files = get_animation_files(dirname .. basename),
         }
      )
   end
   animationtable[animationname] = {
      mipmap = mipmap,
      hotspot = hotspot,
   }
   if (fps ~= nil) then
      animationtable[animationname]["fps"] = fps
   end
end



-- RST
-- .. function:: add_walking_animations(animationtable, animationname, dirname, basename, hotspot [, fps])
--
--    Adds 6 walk or sail animations - one for each walking direction - to `animationtable`.
--    Supports both simple animations and mipmaps.
--    See :ref:`animations` for more documentation and code examples.
--
--    :arg animationtable: The table that the animation data will be added to.
--       It may already contain some animations.
--    :type animationtable: :class:`table`
--    :arg animationname: The name of the animation to be added, e.g. ``walkload``.
--    :type animationname: :class:`string`
--    :arg dirname: The name of the directory where the animation image files are located.
--    :type dirname: :class:`string`
--    :arg basename: The basename of the animation files. The filenames of the
--       animation files need to have the format
--       ``<basename>_(e|ne|se|sw|w|nw)_\d{2,3}.png`` for simple animations, and
--       ``<basename>_(e|ne|se|sw|w|nw)_<scale>_\d{2,3}.png`` for mipmaps.
--       Supported scales are ``0.5``, ``1``, ``2`` and ``4``.
--    :type basename: :class:`string`
--    :arg hotspot: The hotspot coordinates for blitting, e.g. ``{2, 20}``.
--    :type hotspot: :class:`array`
--    :arg fps: Frames per second. Only use this if the animation has more than
--       1 frame, and if you need to deviate from the default frame rate.
--    :type fps: :class:`integer`
function add_walking_animations(animationtable, animationname, dirname, basename, hotspot, fps)
   for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
      if fps ~= nil then
         add_animation(animationtable, animationname .. "_" .. dir, dirname, basename .. "_" .. dir, hotspot, fps)
      else
         add_animation(animationtable, animationname .. "_" .. dir, dirname, basename .. "_" .. dir, hotspot)
      end
   end
end
