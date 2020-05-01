#include "exerciser.h"

void exercise(connection *C)
{
 
  query1(C, 1, 35, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  query1(C, 0, 35, 40, 0, 0, 0, 1, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  query1(C, 0, 35, 40, 0, 0, 0, 0, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  query1(C, 0, 40, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  add_player(C, 1, 1, "abcd", "xyzx", 1, 1, 1, 1, 1, 1);
  add_team(C, "testteam", 10, 3, 20, 0);
  add_state(C, "teststate");
  add_color(C, "testcolor");
  query2(C, "LightBlue");
  query3(C, "NCSU");
  query5(C, 13);
  query2(C, "LightBlue");
  query5(C, 13);
}
