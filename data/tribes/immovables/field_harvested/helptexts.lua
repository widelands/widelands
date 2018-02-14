function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a field
      default = _("This field has been harvested.")
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
