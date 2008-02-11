with Ada.Characters.Latin_1; use Ada.Characters.Latin_1;
with Ada.Command_Line;       use Ada.Command_Line;
with Ada.Exceptions;         use Ada.Exceptions;
with Ada.Integer_Text_IO;    use Ada.Integer_Text_IO;
with Ada.IO_Exceptions;
with Ada.Sequential_IO;
with Ada.Text_IO;            use Ada.Text_IO;
procedure Leading_Whitespace_Checker is
   package Character_IO is new Ada.Sequential_IO (Character); use Character_IO;
   Syntax_Error       : exception;
   The_File           : Character_IO.File_Type;
   Previous_Character : Character;
   Current_Character  : Character := LF; --  So empty file has newline at end.
   Line_Number        : Positive  := 1;

   procedure Next_Character; pragma Inline (Next_Character);
   procedure Next_Character is begin
      Previous_Character := Current_Character;
      Read (The_File, Current_Character);
   end Next_Character;

   procedure Next_Line; pragma Inline (Next_Line);
   procedure Next_Line is begin
      Line_Number := Line_Number + 1;
   end Next_Line;

   procedure Put_Error (Message : in String);
   procedure Put_Error (Message : in String) is begin
      Put      (Argument (1) & ':');
      Put      (Line_Number, Width => 1);
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

   function Read_Code return Integer;
   function Read_Code return Integer is
      Number_Of_Open_Parentheses : Integer   := 0;
   begin
      Read_Code_Loop : loop
         case Current_Character is
            when LF     =>
               exit Read_Code_Loop;
            when '('    =>
               Number_Of_Open_Parentheses := Number_Of_Open_Parentheses + 1;
            when ')'    =>
               Number_Of_Open_Parentheses := Number_Of_Open_Parentheses - 1;
            when '''    =>
               Next_Character;
               case Current_Character is
                  when LF     =>
                     Put_Error ("unterminated character constant");
                     exit Read_Code_Loop;
                  when '''    =>
                     Raise_Exception
                       (Syntax_Error'Identity, "invalid character constant");
                  when '\'    =>
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
                    (Syntax_Error'Identity,
                     "expected closing '\'' of character constant");
               end if;
            when '"'    =>
               loop
                  Next_Character;
                  case Current_Character is
                     when LF     =>
                        Put_Error ("unterminated string constant");
                        exit Read_Code_Loop;
                     when '\'    =>
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
            when '*'    =>
               if Previous_Character = '/' then --  /* comment */
                  Read_Multiline_Comment;
               end if;
            when '/'    =>
               if Previous_Character = '/' then --  // comment
                  loop
                     Next_Character;
                     exit Read_Code_Loop when Current_Character = LF;
                  end loop;
               end if;
            when others =>
               null;
         end case;
         Next_Character;
      end loop Read_Code_Loop;
      if 0 < Number_Of_Open_Parentheses then
         Put_Error ("parenthesis not closed before end of line");
      end if;
      return Number_Of_Open_Parentheses;
   end Read_Code;

   Allowed_Indentation_Increase : Integer := 0;
   Previous_Depth_Indentation   : Natural := 0;
   Previous_Depth_Padding       : Natural := 0;
   Depth_Indentation            : Natural := 0;
   Depth_Padding                : Natural := 0;
   Depth_Parentheses            : Natural := 0;
   Next_Character_Is_Read       : Boolean := False;
begin
   Open (The_File, In_File, Argument (1));
   while Current_Character /= LF or not End_Of_File (The_File) loop
      if not Next_Character_Is_Read then
         Next_Character;
      end if;
      Next_Character_Is_Read := False;
      case Current_Character is
         when LF     =>
            Next_Line;
            Depth_Padding := 0;
         when '#'    =>
            Read_Macro;
         when HT     =>
            Depth_Indentation := Depth_Indentation + 1;
         when ' '    =>
            Depth_Padding     := Depth_Padding     + 1;
         when '('    =>
            Depth_Parentheses := Depth_Parentheses + 1;
            Allowed_Indentation_Increase := 2; --  hack for Nicolai's style
         when others =>
            case Current_Character is
               when '}'                   =>
                  --  Must read the next character already so that we can see
                  --  whether we have "};" which is allowed to be at the same
                  --  indentation level as the previous code line, because it
                  --  may be the end of a namespace. If it is just a '}', the
                  --  indentation depth must decrease.
                  --
                  --  Unfortunately it is also necessary to add ';' after '}'
                  --  in some other rare constructs to avoid a style error
                  --  here, such as extern "C" {}; and some switch statements.
                  Next_Character;
                  if Current_Character = LF then
                     Next_Character_Is_Read := True;
                  end if;
                  if Current_Character = ';' then
                     Allowed_Indentation_Increase :=  0;
                  else
                     Allowed_Indentation_Increase := -1;
                  end if;
               when ')' | ';' =>
                  --  Must allow "()" (empty parameter list) and "(;" for
                  --  statement without loop variable declaration.
                  if Previous_Character /= '(' then
                     Put_Error
                       ("illegal '" & Current_Character &
                        "' at beginning of line");
                  end if;
               when ',' =>
                  --  Must allow ',' at the beginning of a line, for
                  --  situations like:
                  --  some_member(0)
                  --  #ifnded NDEBUG
                  --  , member_that_is_initialized_in_debug_builds_only(0)
                  --  #endif
                  if Previous_Character /= LF then
                     Put_Error ("illegal ',' at beginning of line");
                  end if;
               when ']' =>
                  Put_Error
                    ("illegal '" & Current_Character &
                     "' at beginning of line");
               when others =>
                  null;
            end case;

--            Put_Line
--              ("DEBUG: line number =" & Line_Number'Img &
--               ", Previous_Depth_Indentation =" &
--               Previous_Depth_Indentation'Img &
--               ", allowed indentation increase =" &
--               Allowed_Indentation_Increase'Img);

            if
              Previous_Depth_Indentation + Allowed_Indentation_Increase
              <
              Depth_Indentation
            then
               Put_Error ("indentation is too deep");
            end if;

            if Previous_Depth_Padding < Depth_Padding then
               Put_Error
                 ("too much padding before the first non-whitespace " &
                  "character:" & Depth_Padding'Img);
            end if;

            Previous_Depth_Indentation   := Depth_Indentation;
            Depth_Indentation            := 0;
            declare
               Integer_Previous_Depth_Padding : constant Integer :=
                 Depth_Padding + Depth_Parentheses + Read_Code;
            begin
               if Integer_Previous_Depth_Padding < 0 then
                  Put_Error
                    ("closing parenthesis without sufficient padding");
                  Previous_Depth_Padding := 0;
               else
                  Previous_Depth_Padding := Integer_Previous_Depth_Padding;
               end if;
            end;
            pragma Assert (Current_Character = LF);
            case Previous_Character is
               when ',' | ';' | '}' =>
                  Allowed_Indentation_Increase :=  0;
               when others    =>
                  Allowed_Indentation_Increase :=  1;
            end case;
            if not Next_Character_Is_Read then
               Next_Line;
            end if;
      end case;
   end loop;
exception
   when Ada.IO_Exceptions.End_Error =>
      Put_Error ("unexpected end of file");
   when Error : Syntax_Error        =>
      Put_Error (Exception_Message (Error));
   when others =>
      Put_Error ("INTERNAL ERROR: " & Argument (1) & Line_Number'Img);
end Leading_Whitespace_Checker;
