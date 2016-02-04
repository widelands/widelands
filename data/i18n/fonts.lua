-- This file defines the font sets available for different languages.
--
-- Define which fontset you wish to use for your language in locales.lua.
--
-- When adding a new fontset, also make sure you have all pertinent license
-- and source information collected in its folder.


return {
   -- This is the default set and always needs to be complete. It covers all possible font styles.
   default = {
      -- If your language doesn't distinguish between serif and sans serif fonts, please use the serif set.
       -- Your font set should always define "serif".
      serif = "DejaVu/DejaVuSerif.ttf",
       -- Add bold and italic variants if your font set has them.
      serif_bold = "DejaVu/DejaVuSerif-Bold.ttf",
      serif_italic = "DejaVu/DejaVuSerif-Italic.ttf",
      serif_bold_italic = "DejaVu/DejaVuSerif-BoldItalic.ttf",
      -- If your font set doesn't have sans or condensed variants, serif will be used instead.
      sans = "DejaVu/DejaVuSans.ttf",
      sans_bold = "DejaVu/DejaVuSans-Bold.ttf",
      sans_italic = "DejaVu/DejaVuSans-Oblique.ttf",
      sans_bold_italic = "DejaVu/DejaVuSans-BoldOblique.ttf",
      condensed = "DejaVu/DejaVuSansCondensed.ttf",
      condensed_bold = "DejaVu/DejaVuSansCondensed-Bold.ttf",
      condensed_italic = "DejaVu/DejaVuSansCondensed-Oblique.ttf",
      condensed_bold_italic = "DejaVu/DejaVuSansCondensed-BoldOblique.ttf",
      -- If the diection isn't defined, your font set will default to "ltr" = left-to-right.
      -- For right-to-left or bidirectional (BiDi) languages, use "rtl".
      direction = "ltr",
      size_offset = 0
   },

   arabic = {
      serif = "amiri/amiri-regular.ttf",
      serif_bold = "amiri/amiri-bold.ttf",
      serif_italic = "amiri/amiri-slanted.ttf",
      serif_bold_italic = "amiri/amiri-boldslanted.ttf",
      direction = "rtl",
      size_offset = 4
   },

   cjk = {
      serif = "MicroHei/wqy-microhei.ttc",
   },

   devanagari = {
      serif = "Nakula/nakula.ttf",
      direction = "rtl",
      size_offset = 2
   },

   hebrew = {
      serif = "Culmus/TaameyFrankCLM-Medium.ttf",
      serif_bold = "TaameyFrankCLM-Bold.ttf",
      serif_italic = "Culmus/TaameyFrankCLM-MediumOblique.ttf",
      sans = "Culmus/TaameyFrankCLM-Medium.ttf",
      sans_bold = "TaameyFrankCLM-Bold.ttf",
      sans_italic = "Culmus/TaameyFrankCLM-MediumOblique.ttf",
      direction = "rtl",
      size_offset = 4
   },

   myanmar = {
      serif = "mmrCensus/mmrCensus.ttf",
      size_offset = 2
   },

   sinhala = {
      serif = "Sinhala/lklug.ttf",
   }
}
