--  Detecs errors in leading whitespace and a few other things in C++ source
--  code.
--
--  Checks the beginning of each line, the so called line begin, which consists
--  of 3 parts:
--    1. Indentation:
--         A sequence of horizontal tab characters. Their count is called the
--         indentation depth of that line. Some validation is done for the
--         indentation depth of lines, for example a line with a closing brace
--         must have the same indentation depth as the line with the matching
--         opening brace.
--    2. Alignment:
--         A sequence of space characters. Their count is called the alignment
--         depth of that line. It must equal the depth of open parentheses from
--         preceding lines.
--    3. Opening parentheses:
--         A sequence of '(' or '[' characters. The matching closing
--         parenthesis does not have to be on the same line.
--  The line begin is read in 3 stages. First characters are read until one is
--  found, that is not a horizontal tab. After this stage the indentation depth
--  of the line is known. Then characters are read until one is found, that is
--  not a space. After this stage the alignment depth of the line is known.
--  Then characters are reade until one is found that is not a '(' or '['.
--  After this stage the whole line begin has been read some errors are
--  reported, such as wrong amount of alignment. If a '#' is found (a
--  preprocessor macro), the macro is read and ignored. Otherwise the rest of
--  the line is read by Read_Code.
--
--  Read_Code reads everything that comes after the line begin, until the end
--  of the line. Any opening parenthesis that it finds must be closed on that
--  line. This is checked after the call by checking whether the return value
--  is positive (which means that a parenthesis was opened but not closed). No
--  checks are done whether a '(' or '[' is matched with a ')' a ']' but that
--  is no problem because the compiler will take care of that. However some
--  errors that the compiler detects are also reported here, such as
--  unterminated character or string constants.
--
--  When Read_Code encounters a closing brace, it checks if the indentation
--  depth of the line equals the indentation depth of the line with the
--  matching opening brace. When an opening brace is encountered, information
--  is stored for later checking of the line with the matching closing brace.
--
--  Errors are reported in <filename>:<linenumber>: <message> format so that
--  they are understood by other tools. Therefore you can run this program in
--  for example KDevelop and go to the found errors in the source code with a
--  keyboard shortcut. Just open the project settings and set the build
--  command to something like:
--    for i in $(find -name *.cc); do leading_whitespace_checker $i; done

with Ada.Characters.Latin_1; use Ada.Characters.Latin_1;
with Ada.Command_Line;       use Ada.Command_Line;
with Ada.Exceptions;         use Ada.Exceptions;
with Ada.Integer_Text_IO;    use Ada.Integer_Text_IO;
with Ada.IO_Exceptions;
with Ada.Sequential_IO;
with Ada.Text_IO;            use Ada.Text_IO;
procedure Whitespace_Checker is
   package Character_IO is new Ada.Sequential_IO (Character); use Character_IO;

   type Line_Number       is range 1 .. 2**16 - 1;
   for Line_Number      'Size use 16;
   type Indentation_Level is range 0 .. 2** 8 - 1;
   for Indentation_Level'Size use  8;
   type Parentheses_Level is range 0 .. 2** 8 - 1;
   for Parentheses_Level'Size use  8;
   type Opening_Brace is record
      Line        : Line_Number;
      Indentation : Indentation_Level;
      Parentheses : Parentheses_Level;
   end record;
   for Opening_Brace'Size use 32;
   type Brace_Index is range 0 .. 63;
   Opening_Braces : array (Brace_Index) of Opening_Brace;
   Brace_Level : Brace_Index := 0;

   Giving_Up           : exception;
   The_File            : Character_IO.File_Type;
   Previous_Character  : Character;
   Current_Character   : Character := LF; --  So empty file has newline at end.
   Current_Line_Number : Line_Number       := 1;
   Depth_Indentation   : Indentation_Level := 0;
   Depth_Parentheses   : Parentheses_Level := 0;



   procedure Next_Character; pragma Inline (Next_Character);
   procedure Next_Character is begin
      Previous_Character := Current_Character;
      Read (The_File, Current_Character);
   end Next_Character;

   procedure Next_Line; pragma Inline (Next_Line);
   procedure Next_Line is begin
      if Current_Line_Number = Line_Number'Last then
         Raise_Exception (Giving_Up'Identity, "too many lines");
      end if;
      Current_Line_Number := Current_Line_Number + 1;
   end Next_Line;

   procedure Put_Error
     (Message            : in String;
      Line_Number_To_Put : in Line_Number := Current_Line_Number);
   procedure Put_Error
     (Message            : in String;
      Line_Number_To_Put : in Line_Number := Current_Line_Number)
   is begin
      Put      (Argument (1) & ':');
      Put      (Integer'Val (Line_Number_To_Put), Width => 1);
      Put_Line (": " & Message);
   end Put_Error;

   procedure Read_Multiline_Comment; pragma Inline (Read_Multiline_Comment);
   procedure Read_Multiline_Comment is begin
      --  When this is called, "/*" has just been read. Therefore current
      --  character is '*'. We change that to ' ' so that it does not think
      --  that the comment is over if it reads a '/' immediately.
      Current_Character := ' ';
      loop
         Next_Character;
         case Current_Character is
            when LF     =>
               Next_Line;
            when '/'    =>
               exit when Previous_Character = '*';
            when others =>
               null;
         end case;
      end loop;
   end Read_Multiline_Comment;

   procedure Read_Macro; pragma Inline (Read_Macro);
   procedure Read_Macro is begin
      loop
         Next_Character;
         if Current_Character = LF then
            Next_Line;
            exit when Previous_Character /= '\';
         end if;
      end loop;
   end Read_Macro;

   --  Reads code after line begin. Returns the depth parentheses increase
   --  (should be negative) when a newline is encountered.
   function Read_Code return Integer;
   function Read_Code return Integer is
      Depth_Parentheses_Increase : Integer := 0;
   begin
      Read_Code_Loop : loop
         case Previous_Character is
            when '(' | '[' =>
               if
                 Current_Character = HT  or
                 Current_Character = ' ' or
                 Current_Character = '#' or
                 Current_Character = ':' or
                 Current_Character = '?' or
                 Current_Character = '%' or
                 Current_Character = ','
               then
                  Put_Error
                    ("""(" & Current_Character & """ is not allowed");
               end if;
            when '}'       =>
               if
                 Current_Character /= LF  and
                 Current_Character /= ' ' and
                 Current_Character /= ';' and
                 Current_Character /= ','
               then
                  Put_Error
                    ("""}" & Current_Character & """ is not allowed");
               end if;
            when ','       =>
               if
                 Current_Character /= LF  and
                 Current_Character /= ' '
               then
                  Put_Error
                    ("""," & Current_Character & """ is not allowed");
               end if;
            when others    =>
               null;
         end case;
         case Current_Character is
            when HT        =>
               Put_Error ("indentation is only allowed at line begin");
            when LF        =>
               if Previous_Character = ' ' then
                  Put_Error ("trailing whitespace");
               end if;
               exit Read_Code_Loop;
            when '{'       =>
               if
                 Previous_Character /= LF and
                 Previous_Character /= HT and
                 Previous_Character /= ' '
               then
                  Put_Error ("""" & Previous_Character & "{"" not allowed");
               end if;
               if Brace_Level = Brace_Index'Last then
                  Raise_Exception
                    (Giving_Up'Identity, "too many levels of braces");
               end if;
               Opening_Braces (Brace_Level) :=
                 (Current_Line_Number, Depth_Indentation, Depth_Parentheses);
               Brace_Level := Brace_Level + 1;
            when '}'       =>
               case Previous_Character is
                  when ',' | ' ' | '?' =>
                     Put_Error ("""}" & Current_Character & """ not allowed");
                  when others                =>
                     null;
               end case;
               if Brace_Level = Brace_Index'First then
                  Raise_Exception
                    (Giving_Up'Identity, "unmatched closing brace");
               end if;
               Brace_Level := Brace_Level - 1;
               declare
                  Opening_Depth_Indentation : constant Indentation_Level :=
                    Opening_Braces (Brace_Level).Indentation;
               begin
                  if Depth_Indentation /= Opening_Depth_Indentation then
                     Put_Error
                       ("wrong indentation depth (closing brace):" &
                        Depth_Indentation'Img);
                     Put_Error
                       ("note: should be" & Opening_Depth_Indentation'Img &
                        " (matching opening brace)",
                        Opening_Braces (Brace_Level).Line);
                  end if;
               end;
            when '(' | '[' =>
               Depth_Parentheses_Increase := Depth_Parentheses_Increase + 1;
            when ')' | ']' =>
               Depth_Parentheses_Increase := Depth_Parentheses_Increase - 1;
               case Previous_Character is
                  when ',' | ' ' | ':' | '?' =>
                     Put_Error
                       ("""" & Previous_Character & Current_Character &
                        """ not allowed");
                  when others                =>
                     null;
               end case;
            when '''       =>
               Next_Character;
               case Current_Character is
                  when LF  =>
                     Put_Error ("unterminated character constant");
                     exit Read_Code_Loop;
                  when ''' =>
                     Raise_Exception
                       (Giving_Up'Identity, "invalid character constant");
                  when '\' =>
                     Next_Character;
                     if Current_Character = LF then
                        Put_Error
                          ("unterminated escape sequence in character " &
                           "constant");
                        exit Read_Code_Loop;
                     end if;
                  when others =>
                     null;
               end case;
               Next_Character;
               if Current_Character /= ''' then
                  Raise_Exception
                    (Giving_Up'Identity,
                     "expected closing '\'' of character constant");
               end if;
            when '"'       =>
               loop
                  Next_Character;
                  case Current_Character is
                     when LF  =>
                        Put_Error ("unterminated string constant");
                        exit Read_Code_Loop;
                     when '\' =>
                        Next_Character;
                        if Current_Character = LF then
                           Put_Error
                             ("unterminated escape sequence in string " &
                              "constant constant");
                           exit Read_Code_Loop;
                        end if;
                     when others =>
                        exit when Current_Character = '"';
                  end case;
               end loop;
            when '*'       =>
               if Previous_Character = '/' then --  /* comment */
                  Read_Multiline_Comment;
               end if;
            when '/'       =>
               if Previous_Character = '/' then --  // comment
                  loop
                     Next_Character;
                     exit Read_Code_Loop when Current_Character = LF;
                  end loop;
               end if;
            when ';' | ',' =>
               if
                 Previous_Character = LF or
                 Previous_Character = HT or
                 Previous_Character = ' '
               then
                  Put_Error
                    ("illegal whitespace before '" & Current_Character & ''');
               end if;
            when others    =>
               null;
         end case;
         Next_Character;
      end loop Read_Code_Loop;
      return Depth_Parentheses_Increase;
   end Read_Code;

   Allowed_Indentation_Increase : Indentation_Level := 0;
   Previous_Depth_Indentation   : Indentation_Level := 0;
   Depth_Alignment              : Parentheses_Level := 0;
begin
   Open (The_File, In_File, Argument (1));
   while not End_Of_File (The_File) loop
      declare
         Parentheses_At_Line_Begin : Parentheses_Level := 0;
      begin
         Next_Character;
         while Current_Character = HT loop
            Depth_Indentation := Depth_Indentation + 1;
            Next_Character;
         end loop;
         while Current_Character = ' ' loop
            Depth_Alignment := Depth_Alignment + 1;
            Next_Character;
         end loop;
         while Current_Character = '(' or Current_Character = '[' loop
            Parentheses_At_Line_Begin := Parentheses_At_Line_Begin + 1;
            Next_Character;
            Allowed_Indentation_Increase := 2; --  hack for Nicolai's style
         end loop;
         case Current_Character is
            when LF        =>
               if Previous_Character = ' ' or Previous_Character = HT then
                  Put_Error ("trailing whitespace");
               elsif Previous_Character = '(' or Previous_Character = '[' then
                  Put_Error
                    ("empty '" & Previous_Character & "' at end of line");
               end if;
               Next_Line;
               Depth_Indentation := 0;
               Depth_Alignment   := 0;
            when '#'       =>
               Read_Macro;
               Depth_Indentation := 0;
               Depth_Alignment   := 0;
            when others    =>
--            Put_Line
--              ("DEBUG: line number ="  & Line_Number      'Img &
--               ", Depth_Indentation =" & Depth_Indentation'Img &
--               ", Depth_Parentheses =" & Depth_Parentheses'Img);

               if
                 0 < Depth_Parentheses
                 and then
                 Depth_Indentation /= Previous_Depth_Indentation
               then
                  Put_Error
                    ("wrong indentation level:" & Depth_Indentation'Img &
                     " (should be" & Previous_Depth_Indentation'Img & ')');
               elsif
                 Previous_Depth_Indentation + Allowed_Indentation_Increase
                 <
                 Depth_Indentation
               then
                  Put_Error ("indentation is too deep");
               end if;

               if Depth_Alignment /= Depth_Parentheses then
                  Put_Error
                    ("wrong amount of leading padding:" & Depth_Alignment'Img &
                     " (should be" & Depth_Parentheses'Img & ')');
               end if;
               Depth_Parentheses :=
                 Depth_Parentheses + Parentheses_At_Line_Begin;
               declare
                  Initial_Brace_Level : constant Brace_Index := Brace_Level;
                  Depth_Parentheses_Increase_After_Line_Begin :
                    constant Integer := Read_Code;
                  New_Depth_Parentheses : constant Integer :=
                    Integer'Val (Depth_Parentheses) +
                    Depth_Parentheses_Increase_After_Line_Begin;
               begin
                  pragma Assert (Current_Character = LF);
                  if 0 < Depth_Parentheses_Increase_After_Line_Begin then
                     Put_Error
                       ("parenthesis not closed before end of line must be " &
                        "at line begin");
                  end if;
                  if
                    (0 < Parentheses_At_Line_Begin or 0 < Depth_Alignment)
                    and
                    Initial_Brace_Level < Brace_Level
                  then
                     Put_Error
                       ("opening brace at line with leading padding or "  &
                        "parentheses must be matched by a closing brace " &
                        "on the same line");
                  end if;
                  if New_Depth_Parentheses < 0 then
                     Put_Error ("unmatched parenthesis");
                     Depth_Parentheses := 0;
                  else
                     Depth_Parentheses :=
                       Parentheses_Level'Val (New_Depth_Parentheses);
                  end if;
               end;
               Previous_Depth_Indentation := Depth_Indentation;
               Depth_Indentation          := 0;
               Depth_Alignment            := 0;

               case Previous_Character is
                  when ',' | ';' | '}' =>
                     Allowed_Indentation_Increase :=  0;
                  when others    =>
                     Allowed_Indentation_Increase :=  1;
               end case;
               Next_Line;
         end case;
      end;
   end loop;
exception
   when Ada.IO_Exceptions.End_Error =>
      Put_Error ("unexpected end of file");
   when Ada.IO_Exceptions.Name_Error =>
      Put_Error ("could not open file");
   when Error : Giving_Up        =>
      Put_Error (Exception_Message (Error));
   when others =>
      Put_Error
        ("INTERNAL ERROR: " & Argument (1) & ':' & Current_Line_Number'Img);
end Whitespace_Checker;
