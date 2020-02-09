function immovable_helptext(tribe)
   local helptext = {
      default = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for a resource: Coal
         _("Coal veins contain coal that can be dug up by coal mines."),
         -- TRANSLATORS: Helptext for a resource: Coal
         _("There is only a little bit of coal here."))
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
