// static_widen.hpp: compile-time transformation from "char" to "CharT"

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_STATIC_WIDEN_HPP
#define HAMIGAKI_STATIC_WIDEN_HPP

namespace hamigaki {

template<class CharT, char c>
struct static_widen;

// for all narrow characters
template<char c>struct static_widen<char,c>{static const char value=c;};

// for basic source character set
template<>struct static_widen<wchar_t,' '>{static const wchar_t value=L' ';};
template<>struct static_widen<wchar_t,'\t'>{static const wchar_t value=L'\t';};
template<>struct static_widen<wchar_t,'\v'>{static const wchar_t value=L'\v';};
template<>struct static_widen<wchar_t,'\f'>{static const wchar_t value=L'\f';};
template<>struct static_widen<wchar_t,'\n'>{static const wchar_t value=L'\n';};
template<>struct static_widen<wchar_t,'a'>{static const wchar_t value=L'a';};
template<>struct static_widen<wchar_t,'b'>{static const wchar_t value=L'b';};
template<>struct static_widen<wchar_t,'c'>{static const wchar_t value=L'c';};
template<>struct static_widen<wchar_t,'d'>{static const wchar_t value=L'd';};
template<>struct static_widen<wchar_t,'e'>{static const wchar_t value=L'e';};
template<>struct static_widen<wchar_t,'f'>{static const wchar_t value=L'f';};
template<>struct static_widen<wchar_t,'g'>{static const wchar_t value=L'g';};
template<>struct static_widen<wchar_t,'h'>{static const wchar_t value=L'h';};
template<>struct static_widen<wchar_t,'i'>{static const wchar_t value=L'i';};
template<>struct static_widen<wchar_t,'j'>{static const wchar_t value=L'j';};
template<>struct static_widen<wchar_t,'k'>{static const wchar_t value=L'k';};
template<>struct static_widen<wchar_t,'l'>{static const wchar_t value=L'l';};
template<>struct static_widen<wchar_t,'m'>{static const wchar_t value=L'm';};
template<>struct static_widen<wchar_t,'n'>{static const wchar_t value=L'n';};
template<>struct static_widen<wchar_t,'o'>{static const wchar_t value=L'o';};
template<>struct static_widen<wchar_t,'p'>{static const wchar_t value=L'p';};
template<>struct static_widen<wchar_t,'q'>{static const wchar_t value=L'q';};
template<>struct static_widen<wchar_t,'r'>{static const wchar_t value=L'r';};
template<>struct static_widen<wchar_t,'s'>{static const wchar_t value=L's';};
template<>struct static_widen<wchar_t,'t'>{static const wchar_t value=L't';};
template<>struct static_widen<wchar_t,'u'>{static const wchar_t value=L'u';};
template<>struct static_widen<wchar_t,'v'>{static const wchar_t value=L'v';};
template<>struct static_widen<wchar_t,'w'>{static const wchar_t value=L'w';};
template<>struct static_widen<wchar_t,'x'>{static const wchar_t value=L'x';};
template<>struct static_widen<wchar_t,'y'>{static const wchar_t value=L'y';};
template<>struct static_widen<wchar_t,'z'>{static const wchar_t value=L'z';};
template<>struct static_widen<wchar_t,'A'>{static const wchar_t value=L'A';};
template<>struct static_widen<wchar_t,'B'>{static const wchar_t value=L'B';};
template<>struct static_widen<wchar_t,'C'>{static const wchar_t value=L'C';};
template<>struct static_widen<wchar_t,'D'>{static const wchar_t value=L'D';};
template<>struct static_widen<wchar_t,'E'>{static const wchar_t value=L'E';};
template<>struct static_widen<wchar_t,'F'>{static const wchar_t value=L'F';};
template<>struct static_widen<wchar_t,'G'>{static const wchar_t value=L'G';};
template<>struct static_widen<wchar_t,'H'>{static const wchar_t value=L'H';};
template<>struct static_widen<wchar_t,'I'>{static const wchar_t value=L'I';};
template<>struct static_widen<wchar_t,'J'>{static const wchar_t value=L'J';};
template<>struct static_widen<wchar_t,'K'>{static const wchar_t value=L'K';};
template<>struct static_widen<wchar_t,'L'>{static const wchar_t value=L'L';};
template<>struct static_widen<wchar_t,'M'>{static const wchar_t value=L'M';};
template<>struct static_widen<wchar_t,'N'>{static const wchar_t value=L'N';};
template<>struct static_widen<wchar_t,'O'>{static const wchar_t value=L'O';};
template<>struct static_widen<wchar_t,'P'>{static const wchar_t value=L'P';};
template<>struct static_widen<wchar_t,'Q'>{static const wchar_t value=L'Q';};
template<>struct static_widen<wchar_t,'R'>{static const wchar_t value=L'R';};
template<>struct static_widen<wchar_t,'S'>{static const wchar_t value=L'S';};
template<>struct static_widen<wchar_t,'T'>{static const wchar_t value=L'T';};
template<>struct static_widen<wchar_t,'U'>{static const wchar_t value=L'U';};
template<>struct static_widen<wchar_t,'V'>{static const wchar_t value=L'V';};
template<>struct static_widen<wchar_t,'W'>{static const wchar_t value=L'W';};
template<>struct static_widen<wchar_t,'X'>{static const wchar_t value=L'X';};
template<>struct static_widen<wchar_t,'Y'>{static const wchar_t value=L'Y';};
template<>struct static_widen<wchar_t,'Z'>{static const wchar_t value=L'Z';};
template<>struct static_widen<wchar_t,'0'>{static const wchar_t value=L'0';};
template<>struct static_widen<wchar_t,'1'>{static const wchar_t value=L'1';};
template<>struct static_widen<wchar_t,'2'>{static const wchar_t value=L'2';};
template<>struct static_widen<wchar_t,'3'>{static const wchar_t value=L'3';};
template<>struct static_widen<wchar_t,'4'>{static const wchar_t value=L'4';};
template<>struct static_widen<wchar_t,'5'>{static const wchar_t value=L'5';};
template<>struct static_widen<wchar_t,'6'>{static const wchar_t value=L'6';};
template<>struct static_widen<wchar_t,'7'>{static const wchar_t value=L'7';};
template<>struct static_widen<wchar_t,'8'>{static const wchar_t value=L'8';};
template<>struct static_widen<wchar_t,'9'>{static const wchar_t value=L'9';};
template<>struct static_widen<wchar_t,'_'>{static const wchar_t value=L'_';};
template<>struct static_widen<wchar_t,'{'>{static const wchar_t value=L'{';};
template<>struct static_widen<wchar_t,'}'>{static const wchar_t value=L'}';};
template<>struct static_widen<wchar_t,'['>{static const wchar_t value=L'[';};
template<>struct static_widen<wchar_t,']'>{static const wchar_t value=L']';};
template<>struct static_widen<wchar_t,'#'>{static const wchar_t value=L'#';};
template<>struct static_widen<wchar_t,'('>{static const wchar_t value=L'(';};
template<>struct static_widen<wchar_t,')'>{static const wchar_t value=L')';};
template<>struct static_widen<wchar_t,'<'>{static const wchar_t value=L'<';};
template<>struct static_widen<wchar_t,'>'>{static const wchar_t value=L'>';};
template<>struct static_widen<wchar_t,'%'>{static const wchar_t value=L'%';};
template<>struct static_widen<wchar_t,':'>{static const wchar_t value=L':';};
template<>struct static_widen<wchar_t,';'>{static const wchar_t value=L';';};
template<>struct static_widen<wchar_t,'.'>{static const wchar_t value=L'.';};
template<>struct static_widen<wchar_t,'?'>{static const wchar_t value=L'?';};
template<>struct static_widen<wchar_t,'*'>{static const wchar_t value=L'*';};
template<>struct static_widen<wchar_t,'+'>{static const wchar_t value=L'+';};
template<>struct static_widen<wchar_t,'-'>{static const wchar_t value=L'-';};
template<>struct static_widen<wchar_t,'/'>{static const wchar_t value=L'/';};
template<>struct static_widen<wchar_t,'^'>{static const wchar_t value=L'^';};
template<>struct static_widen<wchar_t,'&'>{static const wchar_t value=L'&';};
template<>struct static_widen<wchar_t,'|'>{static const wchar_t value=L'|';};
template<>struct static_widen<wchar_t,'~'>{static const wchar_t value=L'~';};
template<>struct static_widen<wchar_t,'!'>{static const wchar_t value=L'!';};
template<>struct static_widen<wchar_t,'='>{static const wchar_t value=L'=';};
template<>struct static_widen<wchar_t,','>{static const wchar_t value=L',';};
template<>struct static_widen<wchar_t,'\\'>{static const wchar_t value=L'\\';};
template<>struct static_widen<wchar_t,'"'>{static const wchar_t value=L'"';};
template<>struct static_widen<wchar_t,'\''>{static const wchar_t value=L'\'';};

// for additional basic execution wide-character set
template<>struct static_widen<wchar_t,'\a'>{static const wchar_t value=L'\a';};
template<>struct static_widen<wchar_t,'\b'>{static const wchar_t value=L'\b';};
template<>struct static_widen<wchar_t,'\r'>{static const wchar_t value=L'\r';};
template<>struct static_widen<wchar_t,'\0'>{static const wchar_t value=L'\0';};

} // End namespace hamigaki.

#endif // HAMIGAKI_STATIC_WIDEN_HPP
