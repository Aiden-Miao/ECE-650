#include <iostream>
#include <pqxx/pqxx>
#include <fstream>
#include "exerciser.h"

using namespace std;
using namespace pqxx;

void create_table(connection * C){
  //creat tables
  string sql;
  sql += "DROP TABLE IF EXISTS PLAYER CASCADE;"; 
  sql += "DROP TABLE IF EXISTS TEAM CASCADE;";    
  sql += "DROP TABLE IF EXISTS STATE CASCADE;";  
  sql += "DROP TABLE IF EXISTS COLOR CASCADE;";
  
  sql += "CREATE TABLE COLOR("		    \
   "COLOR_ID SERIAL PRIMARY KEY  NOT NULL," \
   "NAME                    TEXT NOT NULL);";

  sql += "CREATE TABLE STATE("			\
   "STATE_ID SERIAL PRIMARY KEY  NOT NULL,"	\
   "NAME                    TEXT NOT NULL);";

  
  sql += "CREATE TABLE TEAM("		     \
    "TEAM_ID SERIAL PRIMARY KEY   NOT NULL," \
    "NAME                TEXT  NOT NULL," \
    "STATE_ID            INT   NOT NULL," \
    "COLOR_ID            INT   NOT NULL," \
    "WINS                INT   NOT NULL," \
    "LOSSES              INT   NOT NULL,"\
    "CONSTRAINT STATE_FK FOREIGN KEY (STATE_ID) REFERENCES STATE(STATE_ID) ON DELETE SET NULL ON UPDATE CASCADE,"\
    "CONSTRAINT COLOR_FK FOREIGN KEY (COLOR_ID) REFERENCES COLOR(COLOR_ID) ON DELETE SET NULL ON UPDATE CASCADE);";

  sql +=  "CREATE TABLE PLAYER("	    \
    "PLAYER_ID SERIAL PRIMARY KEY   NOT NULL," \
    "TEAM_ID             INT   NOT NULL," \
    "UNIFORM_NUM         INT   NOT NULL," \
    "FIRST_NAME          TEXT   NOT NULL," \
    "LAST_NAME           TEXT   NOT NULL," \
    "MPG                 INT    NOT NULL,"\
    "PPG                 INT    NOT NULL," \
    "RPG                 INT    NOT NULL,"\
    "APG                 INT    NOT NULL," \
    "SPG                 REAL    NOT NULL," \
    "BPG                 REAL   NOT NULL,"\
    "CONSTRAINT TEAM_FK FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID) ON DELETE SET NULL ON UPDATE CASCADE);";

  work W(*C);
  W.exec(sql);
  W.commit();
  //  cout << "Table created successfully" << endl;
}

/*
void drop_table(connection * C){
  string sql;
  sql += "DROP TABLE PLAYER; DROP TABLE TEAM; DROP TABLE STATE; DROP TABLE COLOR;";
  work W(*C);
  W.exec(sql);
  W.commit();
  cout << "Table dropped successfully" << endl;
}
*/
void load_team(string filename, connection * C){
  ifstream infile(filename);
  string line;
  while(std::getline(infile,line)){
    vector<string> token;
    //cout<<line<<endl;
    size_t pos = 0;
    while((pos = line.find(" "))!=string::npos){
      //      cout<<"infinite while loop"<<endl;
      token.push_back(line.substr(0,pos));
      line.erase(0, pos + 1);
    }
    token.push_back(line);
    add_team(C, token[1], stoi(token[2]), stoi(token[3]), stoi(token[4]), stoi(token[5]));
  }
}

void load_player(string filename, connection *C){
  ifstream infile(filename);
  string line;
  while(std::getline(infile,line)){
    vector<string> token;
    //cout<<line<<endl;
    size_t pos = 0;
    while((pos = line.find(" "))!=string::npos){
      //      cout<<"infinite while loop"<<endl;
      token.push_back(line.substr(0,pos));
      line.erase(0, pos + 1);
    }
    token.push_back(line);
    add_player(C, stoi(token[1]), stoi(token[2]), token[3], token[4], stoi(token[5]), stoi(token[6]),stoi(token[7]), stoi(token[8]), stod(token[9]), stod(token[10]));
  }
}

void load_color(string filename, connection * C){
  ifstream infile(filename);
  string line;
  while(std::getline(infile,line)){
    vector<string> token;
    //cout<<line<<endl;
    size_t pos = 0;
    while((pos = line.find(" "))!=string::npos){
      //      cout<<"infinite while loop"<<endl;
      token.push_back(line.substr(0,pos));
      line.erase(0, pos + 1);
    }
    token.push_back(line);
    add_color(C, token[1]);
  }
}

void load_state(string filename, connection * C){
  ifstream infile(filename);
  string line;
  while(std::getline(infile,line)){
    vector<string> token;
    //cout<<line<<endl;
    size_t pos = 0;
    while((pos = line.find(" "))!=string::npos){
      //      cout<<"infinite while loop"<<endl;
      token.push_back(line.substr(0,pos));
      line.erase(0, pos + 1);
    }
    token.push_back(line);
    add_state(C, token[1]);
  }
}

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;
  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      //     cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }
  

  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files

  
  create_table(C);
  load_state("state.txt", C);
  load_color("color.txt", C);
  load_team("team.txt", C);
  load_player("player.txt", C);
  exercise(C);
  //  drop_table(C);

  //Close database connection
  C->disconnect();
  return 0;
}


