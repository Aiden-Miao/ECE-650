#include "query_funcs.h"
#include <string.h>
#include <iomanip>
#include <iostream>
using namespace std;

void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
  work W(*C);  
  string sql;
  sql += "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, PPG, RPG, APG, SPG, BPG) VALUES ("  + to_string(team_id) + ", " + to_string(jersey_num) + ", " +  W.quote(first_name) + ", " + W.quote(last_name) + ", " + to_string(mpg) + ", " + to_string(ppg) + ", " + to_string(rpg) + ", " + to_string(apg) + ", " + to_string(spg) + ", " + to_string(bpg) + ");";
  W.exec(sql);
  W.commit();
  
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
  
  string sql;
  sql += "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES('" + name + "', " + to_string(state_id) + ", " + to_string(color_id) + ", " + to_string(wins) + ", " + to_string(losses) + ");";
  //cout<<sql<<endl;
  work W(*C);
  W.exec(sql);
  W.commit();
  
}


void add_state(connection *C, string name)
{
  
  string sql;
  sql += "INSERT INTO STATE (NAME) VALUES('" + name + "');";
  work W(*C);
  W.exec(sql);
  W.commit();
  
}


void add_color(connection *C, string name)
{
  
  string sql;
  sql += "INSERT INTO COLOR (NAME) VALUES('" + name + "');";
  work W(*C);
  W.exec(sql);
  W.commit();
  
}


void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
  string sql;
  sql += "SELECT * FROM PLAYER";
  int start_count = 0;//start of the selection
  if(use_mpg){
    if(start_count == 0){
      sql += " WHERE ";
    }
    else{
      sql += " AND ";
    }
    sql += "MPG BETWEEN " + to_string(min_mpg) + " AND " + to_string(max_mpg);
    start_count ++;
  }
  if(use_ppg){
    if(start_count == 0){
      sql += " WHERE ";
    }
    else{
      sql += " AND ";
    }
    sql += "PPG BETWEEN " + to_string(min_ppg) + " AND " + to_string(max_ppg);
    start_count ++;
  }
  if(use_rpg){
    if(start_count == 0){
      sql += " WHERE ";
    }
    else{
      sql += " AND ";
    }
    sql += "RPG BETWEEN " + to_string(min_rpg) + " AND " + to_string(max_rpg);
    start_count ++;
  }
  if(use_apg){
    if(start_count == 0){
      sql += " WHERE ";
    }
    else{
      sql += " AND ";
    }
    sql += "APG BETWEEN " + to_string(min_apg) + " AND " + to_string(max_apg);
    start_count ++;
  }
  if(use_spg){
    if(start_count == 0){
      sql += " WHERE ";
    }
    else{
      sql += " AND ";
    }
    sql += "SPG BETWEEN " + to_string(min_spg) + " AND " + to_string(max_spg);
    start_count ++;
  }
  if(use_bpg){
    if(start_count == 0){
      sql += " WHERE ";
    }
    else{
      sql += " AND ";
    }
    sql += "BPG BETWEEN " + to_string(min_bpg) + " AND " + to_string(max_bpg);
    start_count ++;
  }
  sql += ";";
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG\n";
  for(result::const_iterator c = R.begin(); c!=R.end(); ++c){
    cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>() << " " << c[3].as<string>()
	 << " " << c[4].as<string>() << " " << c[5].as<int>() << " " <<c[6].as<int>() << " " << c[7].as<int>()
	 <<" " << c[8].as<int>() << " " << fixed << setprecision(1)<< c[9].as<double>() << " " << c[10].as<double>() << endl;
  }
}


void query2(connection *C, string team_color)
{
  string sql;
  sql += "SELECT TEAM.NAME FROM TEAM, COLOR WHERE COLOR.COLOR_ID = TEAM.COLOR_ID AND COLOR.NAME = ";
  sql += "'" + team_color + "'" + ";";
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "NAME" <<endl;
  for(result::const_iterator c = R.begin(); c!=R.end(); ++c){
    cout << c[0].as<string>() << endl;
  }
}


void query3(connection *C, string team_name)
{
  string sql;
  sql += "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE TEAM.TEAM_ID = PLAYER.TEAM_ID AND TEAM.NAME = ";
  sql += "'" + team_name  +"' " + "ORDER BY PPG DESC";
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "FIRST_NAME LAST_NAME" <<endl;
  for(result::const_iterator c = R.begin(); c!=R.end(); ++c){
    cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
  }  
}


void query4(connection *C, string team_state, string team_color)
{
  string sql;
  sql += "SELECT FIRST_NAME, LAST_NAME, UNIFORM_NUM FROM PLAYER, STATE, COLOR, TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.STATE_ID = STATE.STATE_ID AND TEAM.COLOR_ID = COLOR.COLOR_ID AND STATE.NAME = ";
  sql += "'" + team_state +"' " +"AND COLOR.NAME = ";
  sql += "'" + team_color +"';";
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" <<endl;
  for(result::const_iterator c = R.begin(); c!=R.end(); ++c){
    cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<int>() << endl;
  }  
}


void query5(connection *C, int num_wins)
{
  string sql;
  sql += "SELECT FIRST_NAME, LAST_NAME, TEAM.NAME, TEAM.WINS FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.WINS > ";
  sql += "'" + to_string(num_wins) +"';";
  nontransaction N(*C);
  result R(N.exec(sql));
  cout << "FIRST_NAME LAST_NAME NAME WINS" <<endl;
  for(result::const_iterator c = R.begin(); c!=R.end(); ++c){
    cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<string>() << " " <<c[3].as<int>() << endl;
  }
}
