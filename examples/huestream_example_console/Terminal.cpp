/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream_example_console/Terminal.h>

#if defined(WIN32)
#include <conio.h>
#else

#include <termios.h>

#endif

#include <stdio.h>


#if defined(WIN32)
char Terminal::GetChar() {
    return _getch();
}
#else

char Terminal::GetChar() {
    static struct termios old, nw;
    char ch;
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    nw = old; /* make new settings same as old settings */
    nw.c_lflag &= ~ICANON; /* disable buffered i/o */
    nw.c_lflag &= ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &nw); /* use these new terminal i/o settings now */
    ch = getchar();
    tcsetattr(0, TCSANOW, &old);
    return ch;
}

#endif
