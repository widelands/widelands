function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a tribe immovable
      default = _("The remains of a destroyed building.")
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
