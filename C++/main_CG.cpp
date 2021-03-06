// prob: https://www.codingame.com/ide/puzzle/search-race

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <algorithm>
#include <cassert>


using namespace std;
#define PI 3.1415926

const int WIDTH=16000;
const int HEIGHT=9000;

float degrees(float ang_rad){
  return ang_rad* 180/PI;
}

float radians(const int& ang){
  return ang*PI/180;
}

int truncate(float f){
  if(f>0)
    return floor(f);
  else
    return ceil(f);
}


template <typename t>
struct Vec2d{
public:
  t x,y;
public:
  Vec2d(){
    x=0,y=0;
  }
  Vec2d(t i_x,t i_y){
    x=i_x;y=i_y;
  }
  const t& dist(const Vec2d<t>& i_vec2d){
    return sqrt((x-i_vec2d.x)^2+(y-i_vec2d.y)^2);
  }

  const float& orientation(){
    // TODO
    return 0;
  }

  Vec2d<t> operator + (const Vec2d<int>& i_vec){
    return Vec2d<t>(x+i_vec.x,y+i_vec.y);
  }
  Vec2d<t> operator + (const Vec2d<float>& i_vec){
    return Vec2d<t>(x+i_vec.x,y+i_vec.y);
  }

  // this multiplier is thrust which is an integer. Hence only using int mul
  Vec2d<t> operator * (const int& mul){
    return Vec2d<t>(x*mul,y*mul);
  }
  void print(){
    cout << "(" << x << "," << y << ")";
  }
};


class Checkpoint{
public:
  Vec2d<int> pos;
  short num;
  short status;// -1 for done, 0 current, 1 for future
public:
  Checkpoint(const Vec2d<int>& i_pos,const short& i_n){
    pos=i_pos;
    num=i_n;
    status=1;
  }

  void setStatus(const short& i_stat){
    status=i_stat;
  }

};

class Car{
public:
  Vec2d<float> pos,vel,target;
  Vec2d<float> dir;
  int thrust;
  int ang_deg;
  int CheckpointIndex;
  bool isAngleSet;
public:
  Car(const Vec2d<float>& i_pos){
    pos=i_pos;
    target=i_pos;
    thrust=0;
    ang_deg=0;
    CheckpointIndex=0;
    isAngleSet=false;
  }

  void update(const Vec2d<float>& i_Target,const int& i_thrust){
    float new_ang_rad=atan2(i_Target.y-pos.y,i_Target.x-pos.x);
    if(isAngleSet){
      float new_ang_deg=degrees(new_ang_rad);
      if(new_ang_deg-ang_deg > 18){
        new_ang_deg=ang_deg+18;
        new_ang_rad=radians(new_ang_deg);
      }
      if(new_ang_deg-ang_deg < -18){
        new_ang_deg=ang_deg-18;
        new_ang_rad=radians(new_ang_deg);
      }
    }
    update(new_ang_rad,i_thrust);
  }

  void update(const int& i_del_ang,const int& i_thrust){
    int new_ang_deg = ang_deg + i_del_ang;
    float new_ang_rad=radians(new_ang_deg);
    update(new_ang_rad,i_thrust);
  }

  void update(const float& i_new_ang_rad,const int& i_thrust){
    dir=Vec2d<float>(cos(i_new_ang_rad),sin(i_new_ang_rad));
    Vec2d<float> accel=Vec2d<float>(dir.x*i_thrust,dir.y*i_thrust);
    Vec2d<float> inter_vel=Vec2d<float>(accel.x + vel.x,accel.y + vel.y);

    // cout << "accel = ";accel.print(); cout << endl;
    // cout << "vel = ";inter_vel.print(); cout << endl;

    Vec2d<float> inter_pos=Vec2d<float>(inter_vel.x + pos.x,inter_vel.y + pos.y);

    inter_vel.x*=0.85;inter_vel.y*=0.85;

    pos.x=truncate(inter_pos.x);pos.y=truncate(inter_pos.y);
    vel.x=truncate(inter_vel.x);vel.y=truncate(inter_vel.y);
    thrust=i_thrust;
    ang_deg=round(degrees(i_new_ang_rad));
    isAngleSet=true;
  }

  const Vec2d<float>& getPos(){
    return pos;
  }

  const Vec2d<float>& getVel(){
    return vel;
  }
  Vec2d<float> getAcc(){
    return dir*thrust;
  }

  void debug(){
    Vec2d<float> acc=getAcc();
    // cout << "Dir: "; dir.print();
    cout << " Pos: "; pos.print();
    cout << " Vel: "; vel.print();
    cout << " Ang: " << ang_deg;
    cout << endl;

  }

};

vector<Checkpoint> v_CP;
// int CPindex=0;

float dist(const Vec2d<float>& P1,const Vec2d<float>& P2){
  return sqrt(pow(P1.x-P2.x,2) + pow(P1.y-P2.y,2));
}
float dist(const Vec2d<float>& P1,const Vec2d<int>& P2){
  return sqrt(pow(P1.x-P2.x,2) + pow(P1.y-P2.y,2));
}


int boolstr(string s){
    int val=0;
    int powerof2=1;
    for(int i=s.length()-1;i>=0;--i){
        if(s[i]=='1')
            val+=powerof2;
        powerof2*=2;
    }
    return val;
};

// initialization
const int NMOVES=10;
const int BITSPERACTION=11; // 6 for thrust, 5 for angle. Thrust 1st, Angle 2nd
const int POPSIZE=30;
const float MUTATIONPROB=0.3;
const int MAXGENERATIONS=10;

// Thrust range = 0-200, DelAngle range = -18 - 18
// Following bit encryption gives [0:3:189] thruststr, [-15:1:15] angle change
const int THRUSTPORTION=6;
const int ANGLEPORTION=5;

std::default_random_engine generator(0);
std::uniform_int_distribution<int> udist_i(1,NMOVES*BITSPERACTION);
std::uniform_real_distribution<float> udist_r(0.0,1.0);

// GA functions
pair<string,string> crossover(const string& parent1,const string& parent2,float r1,float r2){
    int l=parent1.length();

    // if p1=ab, p2=cd then children are ad,cb
    // inevent of mutation, middle bit is flipped
    string child1=parent1;
    child1.replace(child1.begin()+0.5*l,child1.end(),parent2.begin()+0.5*l,parent2.end());
    if(r1<MUTATIONPROB){
        if(child1[0.5*l]=='0')
            child1[0.5*l]='1';
        else
            child1[0.5*l]='0';
    }

    string child2=parent2;
    child2.replace(child2.begin()+0.5*l,child2.end(),parent1.begin()+0.5*l,parent1.end());
    if(r2<MUTATIONPROB){
        if(child2[0.5*l]=='0')
            child2[0.5*l]='1';
        else
            child2[0.5*l]='0';
    }

    return pair<string,string>{child1,child2};
}


float evaluateactions(string actionset,Car iC){
    vector<pair<int,int>> actions;
    float actionscost=0;

    // decrypting chromosomes to actions
    while(actionset.size()>0){
        string thruststr=actionset.substr(0,THRUSTPORTION);
        actionset.erase(0,THRUSTPORTION);

        // cerr << "here2" << endl;
        assert(thruststr.size()==THRUSTPORTION);

        string anglestr=actionset.substr(0,ANGLEPORTION);
        actionset.erase(0,ANGLEPORTION);
        // cerr << "here3" << endl;

        assert(anglestr.size()==ANGLEPORTION);

        int thrust = 3*boolstr(thruststr);

        int anglechange=boolstr(anglestr.substr(1,4));
        if(anglestr[0]=='1')
            anglechange*=-1;

        actions.emplace_back(thrust,anglechange);
    }

    // Evaluating sequence of actions
    for(const pair<int,int> act:actions){
        int thrust=act.first;int del_ang_deg=act.second;
        iC.update(del_ang_deg,thrust);
        // actionscost+=evaluatestate(iC.pos,iC.CheckpointIndex);

        float d = dist(iC.pos,v_CP[iC.CheckpointIndex].pos);
        while (d <= 600){
          iC.CheckpointIndex+=1;
          d = dist(iC.pos,v_CP[iC.CheckpointIndex].pos);
        }
        actionscost+=d/(iC.CheckpointIndex+1);

    }

    return actionscost;
}


int main(){

  int checkpoints; // Count of checkpoints to read
  cin >> checkpoints; cin.ignore();

  // vector<Checkpoint> cp(checkpoints);

  for (int i = 0; i < checkpoints; i++) {
      int checkpointX; // Position X
      int checkpointY; // Position Y
      cin >> checkpointX >> checkpointY; cin.ignore();
      v_CP.emplace_back(Checkpoint(Vec2d<int>(checkpointX,checkpointY),i));
  }

  // game loop
  while (1) {
      int checkpointIndex; // Index of the checkpoint to lookup in the checkpoints input, initially 0
      int x; // Position X
      int y; // Position Y
      int vx; // horizontal speed. Positive is right
      int vy; // vertical speed. Positive is downwards
      int angle; // facing angle of this car
      cin >> checkpointIndex >> x >> y >> vx >> vy >> angle; cin.ignore();

      string bestcourse="";
      // cout << cp[checkpointIndex].pos.x << " " << cp[checkpointIndex].pos.y << " " << 100;
      // cout << " debug" << endl;


      if (bestcourse.empty()){
          Car GA_Car(Vec2d<float>(x,y));
          GA_Car.vel=Vec2d<float>(vx,vy);
          GA_Car.ang_deg=angle;

          // Initializing all chromosomes to 0
          auto start = chrono::high_resolution_clock::now();

          vector<string> iPop(POPSIZE,string(NMOVES*BITSPERACTION,'0'));

          // This changes the chromosomes to have 50% of 1s at random positions for all members in the population
          for_each(iPop.begin(),iPop.end(),[&](string& actions){
              for(int __=1;__ <=0.5*NMOVES*BITSPERACTION;++__){
                  int setpos=udist_i(generator);
                  actions[setpos]='1';
              }
          });

          sort(iPop.begin(),iPop.end(),[=](const string& actionset_l,const string& actionset_r){
              return evaluateactions(actionset_l,GA_Car) < evaluateactions(actionset_r,GA_Car);
          });

          for(int ___=1;___<=MAXGENERATIONS;++___){
              for(int i=0;i<0.5*POPSIZE;++i){
                  pair<string,string> children=crossover(iPop[i],iPop[0.5*POPSIZE-1-i],udist_r(generator),udist_r(generator));
                  iPop.push_back(children.first);
                  iPop.push_back(children.second);
              };

              sort(iPop.begin(),iPop.end(),[=](const string& actionset_l,const string& actionset_r){
                  return evaluateactions(actionset_l,GA_Car)<evaluateactions(actionset_r,GA_Car);
              });
              iPop.resize(POPSIZE);
          }

          auto stop = chrono::high_resolution_clock::now();
          auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

          // To get the value of duration use the count()
          // member function on the duration object
          cerr << "Execution time = " << duration.count() << "ms" << endl;

          bestcourse=iPop.front();
      }

      cerr << bestcourse.substr(0,THRUSTPORTION+ANGLEPORTION) << endl;

      Car Game_Car(Vec2d<float>(x,y));
      Game_Car.vel=Vec2d<float>(vx,vy);
      Game_Car.ang_deg=angle;

      if (!bestcourse.empty()){
        string thruststr=bestcourse.substr(0,THRUSTPORTION);
        bestcourse.erase(0,THRUSTPORTION);

        string anglestr=bestcourse;

        int thrust = 3*boolstr(thruststr);

        // there are 5 bits for angle. 0 index is for sign, other are value from 0-15
        int anglechange=boolstr(anglestr.substr(1,4));
        if(anglestr[0]=='1')
            anglechange*=-1;

        Game_Car.update(anglechange,thrust);
        cout << Game_Car.pos.x << " " << Game_Car.pos.y << " " << thrust << endl;
      }
      else{
        cout << Game_Car.pos.x << " " << Game_Car.pos.y << " " << 10 << endl;
      }


      // cerr << "(" << x << " " << y << ") ";
      // cerr << "(" << vx << " "<< vy << ") ";
      // cerr << angle << endl;
  }

/*
  Car GA_Car(Vec2d<float>(10353,1986));
  for(int i=0;i<20;++i){
    GA_Car.update(Vec2d<float>(2757,4659),100);
    GA_Car.debug();
  }
*/

  return 0;
};
