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

// Astar algorithm or Beam Search (becoz you are only expanding limited nodes)
//    Create a priority queue of all angle, thrust combinations = 200*37 = 7400
//    Advance the car in each of them
//    score = distance to target - distance travelled
//    reorder the queue in decreasing order of score
//    resize queue to 100?
//    for each of the 100 positions, evaluate the 7400 combinations, score, reorder and resize
//    repeat

// A C++ priority queue cannot be resized
// I need a sorted container that I can access on both ends. so that  when the contained reaches the size limit, I can
// compare the latest value with the worst in the container, delete the last elem and push the new elem into its place.
// A work around on using priority queue : push to a priority queue 100*7400 items, eval them
// pop the top 100 - in a loop!! and repeat
// better, use a vector, push all elems, sort and resize

float astarscore(const Car& c,const Vec2d<int>& startpos,const Vec2d<int>& destpos){
  return dist(c.pos,destpos) - dist(c.pos,startpos);
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

  }

  vector<pair<int,int>> angthrustpairs(7400);
  int index=0;
  for(int angleval=-18;angleval<=18;++angleval){
    for(int thrustval=0;thrustval<=200;++thrustval){
      angthrustpairs[index] = pair<int,int>(angleval,thrustval);
      ++index;
    }
  }

  Vec2d<int> StartPos = v_CP[0];
  Vec2d<int> DestPos = v_CP[CheckpointIndex];
  // Beam Search
  vector<Car> vBeam;
  for (int i=0;i<angthrustpairs.size();++i){
    Car c(StartPos);
    c.update(angthrustpairs[i].first,angthrustpairs[i].second);
    vBeam.push_back(c);
  }

  sort(vBeam.begin(),vBeam.end(),[&](const Car& lcar,const Car& rcar){
    return astarscore(lcar,StartPos,DestPos) < astarscore(rcar,StartPos,DestPos);
  });

  vBeam.resize(100);
  
  for (int step=1;step<=3;++step){
      for(int carinx=0;carinx<100;++carinx){

        for (int i=0;i<angthrustpairs.size();++i){
          Car c2=vBeam[carinx];
          c2.update(angthrustpairs[i].first,angthrustpairs[i].second);
          vBeam.push_back(c2);
        }

      }
      vBeam.erase(vBeam.begin(),vBeam.begin()+100);

      sort(vBeam.begin(),vBeam.end(),[&](const Car& lcar,const Car& rcar){
        return astarscore(lcar,StartPos,DestPos) < astarscore(rcar,StartPos,DestPos);
      });

      vBeam.resize(100);
  }



  return 0;
};
