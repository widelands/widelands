--  Detecs errors in whitespace and a few other things in C++ source code.
--
--  Checks the beginning of each line, the so called line begin, which consists
--  of 2 parts in order:
--    1. Whitespace:
--         A sequence of characters in the set {horizontal tab, space}.
--    2. Opening parentheses:
--         A sequence of characters in the set {'(', '['}. Their matching
--         closing parentheses do not have to be on the same line.
--  A whitespace character in the line begin must be a space if and only if the
--  Character at the same index in the previous line is also in the line begin
--  and is a space or opening parenthesis. (Otherwise it must be a tab.)
--
--  A line with a closing brace must have identical line begin as the line with
--  the matching opening brace. Any line in between must have a line begin that
--  starts with the line begin of the opening/closing lines (but may be
--  longer).
--
--  If a '#' is found (a preprocessor macro) in the first column, the macro is
--  read and ignored. Otherwise the rest of the line is read by Read_Code.
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
   type Leading_Whitespace_Kind is (Tab, Space);
   for Leading_Whitespace_Kind'Size use 1;
   type Leading_Whitespace_Index is range 0 .. 63;
   for Leading_Whitespace_Index'Size use 8;
   type Leading_Whitespace_Array is
     array (Leading_Whitespace_Index) of Leading_Whitespace_Kind;
   pragma Pack (Leading_Whitespace_Array);
   for Leading_Whitespace_Array'Size use 64;
   Leading_Whitespace : Leading_Whitespace_Array;
   Next_Leading_Whitespace_Index : Leading_Whitespace_Index := 0;
   type Opening_Brace is record
      Line                      : Line_Number;
      Leading_Whitespace_Amount : Leading_Whitespace_Index;
   end record;
   for Opening_Brace'Size use 32;
   type Brace_Index is range 0 .. 63;
   Opening_Braces : array (Brace_Index) of aliased Opening_Brace;
   Next_Brace_Level : Brace_Index := 0;

   Giving_Up           : exception;
   The_File            : Character_IO.File_Type;
   Previous_Character  : Character;
   Current_Character   : Character := LF; --  So empty file has newline at end.
   Current_Line_Number : Line_Number := 1;



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

   procedure Read_Multiline_Comment;
   procedure Read_Multiline_Comment is
      HT_Is_Allowed : Boolean := False;
   begin
      --  When this is called, "/*" has just been read. Therefore current
      --  character is '*'. We change that to ' ' so that it does not think
      --  that the comment is over if it reads a '/' immediately.
      Current_Character := ' ';
      loop
         Next_Character;
         case Current_Character is
            when LF     =>
               Next_Line;
               HT_Is_Allowed := True;
            when HT     =>
               if not HT_Is_Allowed then
                  Put_Error ("indentation is only allowed at line begin");
               end if;
            when ' '    =>
               null;
            when '/'    =>
               exit when Previous_Character = '*';
            when others =>
               HT_Is_Allowed := False;
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
               case Current_Character is
                  when HT | ' ' | '#' | '?' | '%' | ',' =>
                     Put_Error
                       ('"' & Previous_Character & Current_Character &
                        """ is not allowed");
                  when others                                 =>
                     null;
               end case;
            when '}'       =>
               case Current_Character  is
                  when LF | ' ' | ';' | ',' =>
                     null;
                  when others               =>
                     Put_Error
                       ("""}" & Current_Character & """ is not allowed");
               end case;
            when ','       =>
               case Current_Character is
                  when LF | ' ' =>
                     null;
                  when others   =>
                     Put_Error
                       ("""," & Current_Character & """ is not allowed");
               end case;
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
               case Previous_Character is
                  when LF | HT | ' ' =>
                     null;
                  when others        =>
                     Put_Error ("""" & Previous_Character & "{"" not allowed");
               end case;
               if Next_Brace_Level = Brace_Index'Last then
                  Raise_Exception
                    (Giving_Up'Identity, "too many levels of braces");
               end if;
               Opening_Braces (Next_Brace_Level)
                 := (Current_Line_Number, Next_Leading_Whitespace_Index);
               Next_Brace_Level := Next_Brace_Level + 1;
            when '}'       =>
               case Previous_Character is
                  when ',' | '?' =>
                     Put_Error
                       ("""" & Previous_Character & Current_Character &
                        """ not allowed");
                  when others                =>
                     null;
               end case;
               if Brace_Index'First = Next_Brace_Level then
                  Raise_Exception
                    (Giving_Up'Identity, "unmatched closing brace");
               end if;
               Next_Brace_Level := Next_Brace_Level - 1;
               declare
                  Matching_Opening_Brace : constant access Opening_Brace
                    := Opening_Braces (Next_Brace_Level)'Access;
               begin
                  if
                    Next_Leading_Whitespace_Index
                    /=
                    Matching_Opening_Brace.Leading_Whitespace_Amount
                  then
                     Put_Error
                       ("wrong amount of leading whitespace before closing " &
                        "brace:" & Next_Leading_Whitespace_Index'Img);
                     Put_Error
                       ("note: must be"                                      &
                        Matching_Opening_Brace.Leading_Whitespace_Amount'Img &
                        " (matching opening brace)",
                        Matching_Opening_Brace.Line);
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
                  when LF     =>
                     Put_Error ("unterminated character constant");
                     exit Read_Code_Loop;
                  when '''    =>
                     Raise_Exception
                       (Giving_Up'Identity, "invalid character constant");
                  when '\'    =>
                     Next_Character;
                     case Current_Character is
                        when '0' | ''' | '\' | 'n' | 'r' | 't' | 'v' =>
                           null;
                        when others                                  =>
                           Put_Error
                             ("illegal escaped character constant: " &
                              Current_Character'Img);
                           exit Read_Code_Loop when Current_Character = LF;
                     end case;
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
                     when LF     =>
                        Put_Error ("unterminated string constant");
                        exit Read_Code_Loop;
                     when '\'    =>
                        Next_Character;
                        case Current_Character is
                           when '0' | '"' | '\' | 'n' | 'r' | 't' | 'v' =>
                              null;
                           when others                                  =>
                              Put_Error
                                ("illegal escaped character in string " &
                                 "constant: " & Current_Character'Img);
                              exit Read_Code_Loop when Current_Character = LF;
                        end case;
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
                     if Current_Character = HT then
                        Put_Error
                          ("indentation is only allowed at line begin");
                     end if;
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

   Indentation_Increase_Allowed : Indentation_Level := 0;
   Space_In_Leading_Whitespace  : Boolean;
begin
   Open (The_File, In_File, Argument (1));
   while not End_Of_File (The_File) loop
      Space_In_Leading_Whitespace := False;
      declare
         Current_Leading_Whitespace_Index : Leading_Whitespace_Index := 0;
         Indentation_Increase             : Indentation_Level        := 0;
      begin
         Read_Leading_Whitespace : loop
            Next_Character;
            case Current_Character is
               when HT     =>
                  pragma Assert
                    (Current_Leading_Whitespace_Index
                     <=
                     Next_Leading_Whitespace_Index);
                  if
                    Current_Leading_Whitespace_Index
                    =
                    Next_Leading_Whitespace_Index
                  then
                     Indentation_Increase := Indentation_Increase + 1;
                     Leading_Whitespace (Current_Leading_Whitespace_Index)
                       := Tab;
                     Next_Leading_Whitespace_Index
                       := Next_Leading_Whitespace_Index + 1;
                  elsif
                    Leading_Whitespace (Current_Leading_Whitespace_Index)
                    =
                    Space
                  then
                     Put_Error
                       ("leading whitespace character #"               &
                        Leading_Whitespace_Index
                        (Current_Leading_Whitespace_Index + 1)
                        'Img                                           &
                        " is 'HT' but should be ' ' according to the " &
                        "previous line");
                  end if;
               when ' '    =>
                  Space_In_Leading_Whitespace := True;
                  if
                    Current_Leading_Whitespace_Index
                    =
                    Next_Leading_Whitespace_Index
                  then
                     Put_Error
                       ("found ' ' as leading whitespace character #"   &
                        Leading_Whitespace_Index
                        (Current_Leading_Whitespace_Index + 1)
                        'Img                                            &
                        " but only" & Next_Leading_Whitespace_Index'Img &
                        " leading whitespace characters are allowed");
                  elsif
                    Leading_Whitespace (Current_Leading_Whitespace_Index) = Tab
                  then
                     Put_Error
                       ("leading whitespace character #"               &
                        Leading_Whitespace_Index
                        (Current_Leading_Whitespace_Index + 1)
                        'Img                                           &
                        " is ' ' but should be 'HT' according to the " &
                        "previous line");
                  end if;
               when others =>
                  exit Read_Leading_Whitespace;
            end case;
            Current_Leading_Whitespace_Index
              := Current_Leading_Whitespace_Index + 1;
         end loop Read_Leading_Whitespace;
         if Current_Character = '(' or Current_Character = '[' then
            Indentation_Increase_Allowed := 2; --  hack for Nicolai's style
         end if;
         if Indentation_Increase_Allowed < Indentation_Increase then
            Put_Error ("indentation is too deep");
         end if;
         if Current_Character /= LF and Current_Character /= '#' then
            Next_Leading_Whitespace_Index := Current_Leading_Whitespace_Index;
         end if;
      end;
      while Current_Character = '(' or Current_Character = '[' loop
         Leading_Whitespace (Next_Leading_Whitespace_Index) := Space;
         Next_Leading_Whitespace_Index :=  Next_Leading_Whitespace_Index + 1;
         Next_Character;
      end loop;
      case Current_Character is
         when LF        =>
            if    Previous_Character = ' ' or Previous_Character = HT  then
               Put_Error ("trailing whitespace");
            elsif Previous_Character = '(' or Previous_Character = '[' then
               Put_Error ("empty '" & Previous_Character & "' at end of line");
            end if;
            Next_Line;
         when '#'       =>
            Read_Macro;
         when others    =>
            if
              Brace_Index'First < Next_Brace_Level
            then
               declare
                  Matching_Opening_Brace : constant access Opening_Brace
                    := Opening_Braces (Next_Brace_Level - 1)'Access;
               begin
                  if
                    Next_Leading_Whitespace_Index
                    <
                    Matching_Opening_Brace.Leading_Whitespace_Amount
                  then
                     Put_Error
                       ("insuficient amount of leading whitespace:" &
                        Next_Leading_Whitespace_Index'Img);
                     Put_Error
                       ("note: must be at least" &
                        Matching_Opening_Brace.Leading_Whitespace_Amount'Img &
                        " (opening brace)",
                        Matching_Opening_Brace.Line);
                  end if;
               end;
            end if;
            declare
               Initial_Next_Brace_Level : constant Brace_Index
                 := Next_Brace_Level;
               Depth_Parentheses_Increase_After_Line_Begin : Integer
                 := Read_Code;
            begin
               pragma Assert (Current_Character = LF);
               if 0 < Depth_Parentheses_Increase_After_Line_Begin then
                  Put_Error
                    ("parenthesis not closed before end of line must be at " &
                     "line begin");
               end if;
               while Depth_Parentheses_Increase_After_Line_Begin < 0 loop
                  if 0 = Next_Leading_Whitespace_Index then
                     Put_Error
                       ("closing parenthesis without matching leading " &
                        "alignment");
                     exit;
                  end if;
                  Next_Leading_Whitespace_Index
                    := Next_Leading_Whitespace_Index - 1;
                  if
                    Leading_Whitespace (Next_Leading_Whitespace_Index) = Space
                  then
                     Depth_Parentheses_Increase_After_Line_Begin
                       := Depth_Parentheses_Increase_After_Line_Begin + 1;
                  end if;
               end loop;
               if
                 Space_In_Leading_Whitespace
                 and then
                 0 < Next_Leading_Whitespace_Index
                 and then
                 Leading_Whitespace (Next_Leading_Whitespace_Index - 1)
                 =
                 Space
                 and then
                 Initial_Next_Brace_Level < Next_Brace_Level
               then
                  Put_Error
                    ("opening brace at line with leading alignment must be "  &
                     "matched by a closing brace on the same line");
               end if;
            end;

            case Previous_Character is
               when ',' | ';' | '}' =>
                  Indentation_Increase_Allowed :=  0;
               when others          =>
                  Indentation_Increase_Allowed :=  1;
            end case;
            Next_Line;
      end case;
   end loop;
exception
   when Ada.IO_Exceptions.End_Error  =>
      Put_Error ("unexpected end of file");
   when Ada.IO_Exceptions.Name_Error =>
      Put_Error ("could not open file");
   when Error : Giving_Up            =>
      Put_Error (Exception_Message (Error));
   when others                       =>
      Put_Error
        ("INTERNAL ERROR: " & Argument (1) & ':' & Current_Line_Number'Img);
end Whitespace_Checker;
