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

public:
  Car(const Vec2d<float>& i_pos){
    pos=i_pos;
    target=i_pos;
    thrust=0;
    ang_deg=0;
    CheckpointIndex=0;
  }
  void update(const Vec2d<float>& i_Target,const int& i_thrust){
    float new_ang_rad=atan2(i_Target.y-pos.y,i_Target.x-pos.x);
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
    // cout << " Acc: "; acc.print();
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

      // cout << cp[checkpointIndex].pos.x << " " << cp[checkpointIndex].pos.y << " " << 100;
      // cout << " debug" << endl;
  }

  return 0;
};
