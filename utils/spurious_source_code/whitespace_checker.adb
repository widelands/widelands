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
--    for i in $(find -name *.cc); do whitespace_checker $i; done

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
   Current_Line_Number : Line_Number := 1;

   --  Small window of the file. The moste recently read character is always
   --  in Read_Characters (0), the previously read in Read_Characters (1) and
   --  so on. Initialize with LF so that an empty file has newline at end.
   Read_Characters     : array (0 .. 10) of Character := (others => LF);

   type Token is (',', ';', '{', '}', Identifier, Other);
   Previous_Token : Token := ';';
   procedure Set_Previous_Token; pragma Inline (Set_Previous_Token);
   procedure Set_Previous_Token is
   begin
      case Read_Characters (1) is
         when ' '                      =>
            null;
         when ','                      =>
            Previous_Token := ',';
         when ';'                      =>
            Previous_Token := ';';
         when '{'                      =>
            Previous_Token := '{';
         when '}'                      =>
            Previous_Token := '}';
         when 'A' .. 'Z' | 'a' .. 'z'  =>
            if
              (Read_Characters (1) = 'r' and then
               Read_Characters (2) = 'o' and then
               (Read_Characters (3) = ' ' or else
                Read_Characters (3) = HT))
              or else
              (Read_Characters (1) = 'r' and then
               Read_Characters (2) = 'o' and then
               Read_Characters (3) = 'x' and then
               (Read_Characters (4) = ' ' or else
                Read_Characters (4) = HT))
              or else
              (Read_Characters (1) = 'd' and then
               Read_Characters (2) = 'n' and then
               Read_Characters (3) = 'a' and then
               (Read_Characters (4) = ' ' or else
                Read_Characters (4) = HT))
              or else
              (Read_Characters (1) = 't' and then
               Read_Characters (2) = 'o' and then
               Read_Characters (3) = 'n' and then
               (Read_Characters (4) = ' ' or else
                Read_Characters (4) = HT or else
                Read_Characters (4) = '('))
            then
               Previous_Token := Other;
            else
               Previous_Token := Identifier;
            end if;
         when others                   =>
            Previous_Token := Other;
      end case;
   end Set_Previous_Token;

   procedure Next_Character; pragma Inline (Next_Character);
   procedure Next_Character is begin
      for I in reverse 0 .. Read_Characters'Last - 1 loop
         Read_Characters (I + 1) := Read_Characters (I);
      end loop;
      Read (The_File, Read_Characters (0));
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
      --  character is '*'. We change that to '-' so that it does not think
      --  that the comment is over if it reads a '/' immediately.
      Read_Characters (0) := '-';
      loop
         Next_Character;
         case Read_Characters (0) is
            when LF     =>
               if Read_Characters (1) = ' ' then
                  Put_Error ("trailing whitespace");
               end if;
               Next_Line;
               HT_Is_Allowed := True;
            when HT     =>
               if not HT_Is_Allowed then
                  Put_Error ("indentation is only allowed at line begin");
               end if;
            when ' '    =>
               null;
            when '/'    =>
               exit when Read_Characters (1) = '*';
            when others =>
               HT_Is_Allowed := False;
         end case;
      end loop;
   end Read_Multiline_Comment;

   procedure Read_Macro; pragma Inline (Read_Macro);
   procedure Read_Macro is begin
      loop
         Next_Character;
         case Read_Characters (0) is
            when LF     =>
               if Read_Characters (1) = ' ' then
                  Put_Error ("trailing whitespace");
               end if;
               Next_Line;
               exit when Read_Characters (1) /= '\';
            when HT     =>
               Put_Error ("'HT' in macro definition is not allowed");
            when others =>
               null;
         end case;
      end loop;
   end Read_Macro;

   Leading_Parentheses_Count        : Natural := 0;
   Previous_Line_Ended_With_Comment : Boolean := False;

   --  Reads code after line begin. Returns the depth parentheses increase
   --  (should be negative) when a newline is encountered.
   function Read_Code return Integer;
   function Read_Code return Integer is
      Depth_Parentheses_Increase : Integer := 0;
      Did_Report_CPE             : Boolean := False;
      procedure Check_Closing_Parenthesis_Condition;
      pragma Inline (Check_Closing_Parenthesis_Condition);
      procedure Check_Closing_Parenthesis_Condition is
      begin
         if
           Leading_Parentheses_Count + Depth_Parentheses_Increase < 0 and
           not Did_Report_CPE
         then
            Put_Error
              ("found '" & Read_Characters (1) &
               "' but only closing tokens are allowed after closing " &
               "parenthesis with corresponding opening parenthesis " &
               "on an earlier line");
            Did_Report_CPE := True;
         end if;
      end Check_Closing_Parenthesis_Condition;
   begin
      Read_Code_Loop : loop
         case Read_Characters (1) is
            when '(' | '['             =>
               case Read_Characters (0) is
                  when HT | ' ' | '#' | '?' | '%' | ',' =>
                     Put_Error
                       ('"' & Read_Characters (1) & Read_Characters (0) &
                        """ is not allowed");
                  when others                                 =>
                     null;
               end case;
            when '}'                   =>
               case Read_Characters (0)  is
                  when LF | ' ' | ';' | ',' =>
                     null;
                  when others               =>
                     Put_Error
                       ("""}" & Read_Characters (0) & """ is not allowed");
               end case;
            when ','                   =>
               case Read_Characters (0) is
                  when LF | ' ' =>
                     null;
                  when others   =>
                     Put_Error
                       ("""," & Read_Characters (0) & """ is not allowed");
               end case;
            when ';'                   =>
               case Read_Characters (0) is
                  when LF | ' ' | ';' | ')' | '}' | '&' | '*' =>
                     null; --  ";&" and ";*" are already reported elsewhere
                  when others                                 =>
                     Put_Error
                       (""";" & Read_Characters (0) & """ is not allowed");
               end case;
            when '='                   =>
               Check_Closing_Parenthesis_Condition;
               case Read_Characters (0) is
                  when LF | ' ' | '=' =>
                     null;
                  when others         =>
                     Put_Error
                       ("""=" & Read_Characters (0) & """ is not allowed");
               end case;
            when '/'                   =>
               case Read_Characters (0) is
                  when LF | '*' | '/'                    =>
                     null;
                  when ' ' | '=' | ',' | ';' | ')' | '}' =>
                     Check_Closing_Parenthesis_Condition;
                  when others                            =>
                     Check_Closing_Parenthesis_Condition;
                     Put_Error
                       ("""/" & Read_Characters (0) & """ is not allowed");
               end case;
            when '-'                   =>
               Check_Closing_Parenthesis_Condition;

               --  Check that there is padding before '-'.
               case Read_Characters (0) is
                  when '-' | '>' =>
                     null; --  Might be -- or ->, so do not require padding.
                  when others    =>
                     case Read_Characters (2) is
                        when HT | ' ' | '-' | '{' | '(' | '[' =>
                           null;
                        when 'e' | 'E'                        =>
                           --  It might be a float with syntax like 1.0e-5,
                           if
                             not (Read_Characters (3) in '0' .. '9') or
                             not (Read_Characters (0) in '0' .. '9')
                           then
                              Put_Error
                                ('"' & Read_Characters (2) & '-' &
                                 Read_Characters (0) & """ is not allowed");
                           end if;
                        when 'r'                              =>
                           --  Might be operator-, operator-= or operator--.
                           if
                             Read_Characters  (3) /= 'o'        or else
                             Read_Characters  (4) /= 't'        or else
                             Read_Characters  (5) /= 'a'        or else
                             Read_Characters  (6) /= 'r'        or else
                             Read_Characters  (7) /= 'e'        or else
                             Read_Characters  (8) /= 'p'        or else
                             Read_Characters  (9) /= 'o'        or else
                             Read_Characters (10)  = '_'        or else
                             Read_Characters (10) in 'A' .. 'Z' or else
                             Read_Characters (10) in 'a' .. 'z' or else
                             Read_Characters (10) in '0' .. '9'
                           then
                              Put_Error
                                ("""r-" & Read_Characters (0) &
                                 " is not allowed");
                           end if;
                        when others                           =>
                           Put_Error
                             ('"' & Read_Characters (2) & '-' &
                              Read_Characters (0) & """ is not allowed");
                     end case;
               end case;
            when '+'                   =>
               Check_Closing_Parenthesis_Condition;

               --  Check that there is padding before '+', unless it is ++.
               if Read_Characters (0) /= '+' then
                  case Read_Characters (2) is
                     when HT | ' ' | '+' =>
                        null;
                     when 'r'                              =>
                        --  Might be operator+, operator+= or operator++.
                        if
                          Read_Characters  (3) /= 'o'        or else
                          Read_Characters  (4) /= 't'        or else
                          Read_Characters  (5) /= 'a'        or else
                          Read_Characters  (6) /= 'r'        or else
                          Read_Characters  (7) /= 'e'        or else
                          Read_Characters  (8) /= 'p'        or else
                          Read_Characters  (9) /= 'o'        or else
                          Read_Characters (10)  = '_'        or else
                          Read_Characters (10) in 'A' .. 'Z' or else
                          Read_Characters (10) in 'a' .. 'z' or else
                          Read_Characters (10) in '0' .. '9'
                        then
                           Put_Error
                             ("""r+" & Read_Characters (0) &
                              " is not allowed");
                        end if;
                     when others         =>
                        Put_Error
                          ('"' & Read_Characters (2) & Read_Characters (1) &
                           Read_Characters (0) & """ is not allowed");
                  end case;
               end if;

               --  Check padding after '+' (ignore the useless unary +).
               if Read_Characters (2) /= '+' then
                  case Read_Characters (0) is
                     when LF | ' ' | '=' | '+' =>
                        null;
                     when others               =>
                        Put_Error
                          ('"' & Read_Characters (2) & '+' &
                           Read_Characters (0) & """ is not allowed");
                  end case;
               end if;
            when '%' | '^'             =>
               Check_Closing_Parenthesis_Condition;

               --  Check padding after the operator.
               case Read_Characters (0) is
                  when LF | ' ' | '=' =>
                     null;
                  when others         =>
                     Put_Error
                       ('"' & Read_Characters (1) & Read_Characters (0) &
                        """ is not allowed");
               end case;
            when ')' | ']' | ' '       =>
               null;
            when others                =>
               Check_Closing_Parenthesis_Condition;
         end case;
         case Read_Characters (0) is
            when ' '       =>
               Set_Previous_Token;
            when HT        =>
               Put_Error ("indentation is only allowed at line begin");
            when LF              =>
               Set_Previous_Token;
               if Read_Characters (1) = ' ' then
                  Put_Error ("trailing whitespace");
               end if;
               exit Read_Code_Loop;
            when '{'       =>
               case Read_Characters (1) is
                  when LF | HT | ' ' =>
                     null;
                  when others        =>
                     Put_Error
                       ("""" & Read_Characters (1) & "{"" not allowed");
               end case;
               if Next_Brace_Level = Brace_Index'Last then
                  Raise_Exception
                    (Giving_Up'Identity, "too many levels of braces");
               end if;
               Opening_Braces (Next_Brace_Level)
                 := (Current_Line_Number, Next_Leading_Whitespace_Index);
               Next_Brace_Level := Next_Brace_Level + 1;
            when '}'       =>
               case Read_Characters (1) is
                  when ',' | '?' =>
                     Put_Error
                       ("""" & Read_Characters (1) & Read_Characters (0) &
                        """ is not allowed");
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
               case Read_Characters (1) is
                  when ',' | ' ' | ':' | '?' =>
                     Put_Error
                       ("""" & Read_Characters (1) & Read_Characters (0) &
                        """ not allowed");
                  when others                =>
                     null;
               end case;
            when '''       =>
               Next_Character;
               case Read_Characters (0) is
                  when LF     =>
                     Put_Error ("unterminated character constant");
                     exit Read_Code_Loop;
                  when '''    =>
                     Raise_Exception
                       (Giving_Up'Identity, "invalid character constant");
                  when '\'    =>
                     Next_Character;
                     case Read_Characters (0) is
                        when
                          '0' | '1' | '2' | ''' | '\' | 'n' | 'r' | 't' | 'v'
                          =>
                           null;
                        when
                          others
                          =>
                           Put_Error
                             ("illegal escaped character constant: " &
                              Read_Characters (0)'Img);
                           exit Read_Code_Loop when Read_Characters (0) = LF;
                     end case;
                  when others =>
                     null;
               end case;
               Next_Character;
               if Read_Characters (0) /= ''' then
                  Raise_Exception
                    (Giving_Up'Identity,
                     "expected closing '\'' of character constant");
               end if;
            when '"'       =>
               loop
                  Next_Character;
                  case Read_Characters (0) is
                     when LF     =>
                        Put_Error ("unterminated string constant");
                        exit Read_Code_Loop;
                     when '\'    =>
                        Next_Character;
                        case Read_Characters (0) is
                           when
                             '0' | '1' | '2' | '"' | '\' | 'n' | 'r' | 't' |
                             'v'
                             =>
                              null;
                           when
                             others
                             =>
                              Put_Error
                                ("illegal escaped character in string " &
                                 "constant: " & Read_Characters (0)'Img);
                              exit Read_Code_Loop when
                                Read_Characters (0) = LF;
                        end case;
                     when others =>
                        exit when Read_Characters (0) = '"';
                  end case;
               end loop;
            when '*'       =>
               case Read_Characters (1) is
                  when '/' => --  /* comment */
                     Read_Multiline_Comment;
                  when
                    HT | ' ' | '{' | '(' | '.' | ':' | '>' | '*' | '&' | '!' =>
                     null;
                  when
                    '+' | '-'                                                =>
                     if Read_Characters (2) /= Read_Characters (1) then
                        Put_Error
                          ('"' & Read_Characters (2) & Read_Characters (1) &
                           "*"" is not allowed");
                     end if;
                  when --  allow "r*" for "operator*"
                    'r'                                                      =>
                     if
                        Read_Characters (2) /= 'o'        or else
                        Read_Characters (3) /= 't'        or else
                        Read_Characters (4) /= 'a'        or else
                        Read_Characters (5) /= 'r'        or else
                        Read_Characters (6) /= 'e'        or else
                        Read_Characters (7) /= 'p'        or else
                        Read_Characters (8) /= 'o'        or else
                        Read_Characters (9)  = '_'        or else
                        Read_Characters (9) in 'A' .. 'Z' or else
                        Read_Characters (9) in 'a' .. 'z' or else
                        Read_Characters (9) in '0' .. '9'
                     then
                        Put_Error ("""r*"" is not allowed");
                     end if;
                  when
                    others                                                   =>
                     Put_Error
                       ('"' & Read_Characters (1) & "*"" is not allowed");
               end case;
            when '/'       =>
               case Read_Characters (1) is
                  when '/' => --  // comment
                     Previous_Line_Ended_With_Comment := True;
                     loop
                        Next_Character;
                        if Read_Characters (0) = HT then
                           Put_Error
                             ("indentation is only allowed at line begin");
                        end if;
                        if Read_Characters (0) = LF then
                           if Read_Characters (1) = ' ' then
                              Put_Error ("trailing whitespace");
                           end if;
                           exit Read_Code_Loop;
                        end if;
                     end loop;
                  when LF | HT | ' ' =>
                     null;
                  when 'r'                              =>
                     --  Might be operator/ or operator/=
                     if
                       Read_Characters (2) /= 'o'        or else
                       Read_Characters (3) /= 't'        or else
                       Read_Characters (4) /= 'a'        or else
                       Read_Characters (5) /= 'r'        or else
                       Read_Characters (6) /= 'e'        or else
                       Read_Characters (7) /= 'p'        or else
                       Read_Characters (8) /= 'o'        or else
                       Read_Characters (9)  = '_'        or else
                       Read_Characters (9) in 'A' .. 'Z' or else
                       Read_Characters (9) in 'a' .. 'z' or else
                       Read_Characters (9) in '0' .. '9'
                     then
                        Put_Error ("""r/"" is not allowed");
                     end if;
                  when others   =>
                     Put_Error
                       ('"' & Read_Characters (1) & "/"" is not allowed");
               end case;
            when ';' | ',' =>
               case Read_Characters (1) is
                  when LF | HT | ' ' =>
                     Put_Error
                       ("illegal whitespace before '" & Read_Characters (0) &
                        ''');
                  when others        =>
                     null;
               end case;
            when '='       =>
               case Read_Characters (1) is
                  when
                    HT  | ' ' | '=' | '+' | '-' | '*' | '/' | '%' | '<' | '>' |
                    '!' | '&' | '|' | '^' | '~' =>
                     null;
                  when 'r'    => --  allow "r(" for "operator=("
                     if
                        Read_Characters (2) /= 'o'        or else
                        Read_Characters (3) /= 't'        or else
                        Read_Characters (4) /= 'a'        or else
                        Read_Characters (5) /= 'r'        or else
                        Read_Characters (6) /= 'e'        or else
                        Read_Characters (7) /= 'p'        or else
                        Read_Characters (8) /= 'o'        or else
                        Read_Characters (9)  = '_'        or else
                        Read_Characters (9) in 'A' .. 'Z' or else
                        Read_Characters (9) in 'a' .. 'z' or else
                        Read_Characters (9) in '0' .. '9'
                     then
                        Put_Error ("""r="" is not allowed");
                     end if;
                  when others =>
                     Put_Error
                       ('"' & Read_Characters (1) & "="" is not allowed");
               end case;
            when '%' | '^' =>
               --  Check that there is padding before the operator.
               case Read_Characters (1) is
                  when HT | ' ' =>
                     null;
                  when 'r'                              =>
                     --  Might be operatorX or operatorX=
                     if
                       Read_Characters (2) /= 'o'        or else
                       Read_Characters (3) /= 't'        or else
                       Read_Characters (4) /= 'a'        or else
                       Read_Characters (5) /= 'r'        or else
                       Read_Characters (6) /= 'e'        or else
                       Read_Characters (7) /= 'p'        or else
                       Read_Characters (8) /= 'o'        or else
                       Read_Characters (9)  = '_'        or else
                       Read_Characters (9) in 'A' .. 'Z' or else
                       Read_Characters (9) in 'a' .. 'z' or else
                       Read_Characters (9) in '0' .. '9'
                     then
                        Put_Error
                          ("""r" & Read_Characters (0) & """ is not allowed");
                     end if;
                  when others         =>
                     Put_Error
                       ('"' & Read_Characters (1) & Read_Characters (0) &
                        """ is not allowed");
               end case;
            when '&'       =>
               case Read_Characters (1) is
                  when HT | ' ' | '(' | '[' | '&' =>
                     null;
                  when others                     =>
                     Put_Error
                       ('"' & Read_Characters (1) & Read_Characters (0) &
                        """ is not allowed");
               end case;
            when '!'       =>
               case Read_Characters (1) is
                  when --  allow "(!" for "(!c)" (! used as ¬) and "!!"
                    HT  | ' ' | '(' | '!' =>
                     null;
                  when --  allow "r!" for "operator!="
                    'r'                   =>
                     if
                        Read_Characters (2) /= 'o'        or else
                        Read_Characters (3) /= 't'        or else
                        Read_Characters (4) /= 'a'        or else
                        Read_Characters (5) /= 'r'        or else
                        Read_Characters (6) /= 'e'        or else
                        Read_Characters (7) /= 'p'        or else
                        Read_Characters (8) /= 'o'        or else
                        Read_Characters (9)  = '_'        or else
                        Read_Characters (9) in 'A' .. 'Z' or else
                        Read_Characters (9) in 'a' .. 'z' or else
                        Read_Characters (9) in '0' .. '9'
                     then
                        Put_Error ("""r!"" is not allowed");
                     end if;
                  when
                    others                =>
                     Put_Error
                       ('"' & Read_Characters (1) & "!"" is not allowed");
               end case;
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
      Leading_Parentheses_Count   := 0;
      Space_In_Leading_Whitespace := False;
      declare
         Current_Leading_Whitespace_Index : Leading_Whitespace_Index := 0;
         Indentation_Increase             : Indentation_Level        := 0;
      begin
         Read_Leading_Whitespace : loop
            Next_Character;
            case Read_Characters (0) is
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
                  pragma Assert
                    (Current_Leading_Whitespace_Index
                     <=
                     Next_Leading_Whitespace_Index);
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
                     Leading_Whitespace (Current_Leading_Whitespace_Index)
                       := Space;
                     Next_Leading_Whitespace_Index
                       := Next_Leading_Whitespace_Index + 1;
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
         if Read_Characters (0) = '(' or Read_Characters (0) = '[' then
            Indentation_Increase_Allowed := 2; --  hack for Nicolai's style
            if
              not Previous_Line_Ended_With_Comment and then
              Previous_Token = Identifier          and then
              Indentation_Increase < 1
            then
               Put_Error ("indentation is too shallow");
            end if;
         end if;
         if Indentation_Increase_Allowed < Indentation_Increase then
            Put_Error ("indentation is too deep");
         end if;
         if Read_Characters (0) /= LF and Read_Characters (0) /= '#' then
            Next_Leading_Whitespace_Index := Current_Leading_Whitespace_Index;
         end if;
      end;
      while Read_Characters (0) = '(' or Read_Characters (0) = '[' loop
         Leading_Whitespace (Next_Leading_Whitespace_Index) := Space;
         Next_Leading_Whitespace_Index := Next_Leading_Whitespace_Index + 1;
         Leading_Parentheses_Count     := Leading_Parentheses_Count     + 1;
         Next_Character;
      end loop;
      case Read_Characters (0) is
         when LF        =>
            if    Read_Characters (1) = ' ' or Read_Characters (1) = HT  then
               Put_Error ("trailing whitespace");
            elsif Read_Characters (1) = '(' or Read_Characters (1) = '[' then
               Put_Error
                 ("empty '" & Read_Characters (1) & "' at end of line");
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
               pragma Assert (Read_Characters (0) = LF);
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

            case Read_Characters (1) is
               when ',' | ';' | '}' =>
                  if Previous_Line_Ended_With_Comment then
                     Previous_Line_Ended_With_Comment := False;
                     Indentation_Increase_Allowed := 1;
                  else
                     Indentation_Increase_Allowed := 0;
                  end if;
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
