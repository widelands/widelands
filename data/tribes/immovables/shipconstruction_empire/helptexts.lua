function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ship construction site.
      default = _("A ship is being constructed at this site.")
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
