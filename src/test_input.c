#include <curses.h>
#include <stdio.h>

int main(void)
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, FALSE); // bloquant, pour debug
  curs_set(1);

  printw("Tape une touche (ESC pour quitter)...\n");
  refresh();

  for (;;)
  {
    int ch = getch();
    if (ch == 27)
      break; // ESC = sortie
    printw("ch=%d\n", ch);
    refresh();
  }

  endwin();
  return 0;
}
