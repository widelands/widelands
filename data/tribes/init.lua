-- RST
-- init.lua
-- -----------------
--
-- Tribes initialization
--
-- First the units are registered, then the tribe descriptions are loaded.
--
-- All tribes also have some basic information for the load screens
-- and the editor in ``preload.lua``.

-- ===================================
--    Preload
-- ===================================
-- NOCOM documentation

tribes = wl.Tribes()

print("┏━ Preloading tribes:")
print_loading_message("┗━ took", function()
   function string.ends(haystack, needle)
      return needle == '' or string.sub(haystack, -string.len(needle)) == needle
   end

   local directories = {"tribes"}
   while #directories > 0 do
      local filepath = directories[1]
      table.remove(directories, 1)
      if path.is_directory(filepath) then
         for idx, listed_path in ipairs(path.list_directory(filepath)) do
            if path.is_directory(listed_path) then
               table.insert(directories, listed_path)
            elseif string.ends(listed_path , "names.lua") then
               names_found = true
               include(listed_path)
               for idx, name in ipairs(names) do
                  tribes:register_object {
                     name = name,
                     script = filepath .. "/init.lua",
                  }
               end
            end
         end
      end
   end
end)

-- ===================================
--    Init
-- ===================================

include "tribes/atlanteans.lua"
include "tribes/barbarians.lua"
include "tribes/empire.lua"
include "tribes/frisians.lua"
