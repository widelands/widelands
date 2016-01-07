-- TODO(GunChleoc): This is unused

function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS#: Helptext for a resource: Stones
      atlanteans = "Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine." .. " " .. "There are many precious stones here.",
      -- TRANSLATORS#: Helptext for a resource: Stones
      barbarians = "Granite is a basic building material and can be dug up by a granite mine." .. " " .. "There is a lot of granite here.",
      -- TRANSLATORS#: Helptext for a resource: Stones
      empire = "Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine." .. " " .. "There is a lot of marble here."
   }
   local result = ""
   if tribe then
      result = helptext[tribe]
   else
      result = helptext["default"]
   end
   if (result == nil) then result = "" end
   return result
end
