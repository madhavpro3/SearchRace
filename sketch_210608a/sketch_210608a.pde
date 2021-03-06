// prob: https://www.codingame.com/ide/puzzle/search-race

float SCALE=0.1;
PImage bg;
Car c;
Checkpoint cp;
int COUNTER=0;

void setup(){
  size(1600,900);
  bg=loadImage("../img/road_1600x900.jpg");
  
  c=new Car(new PVector(1035.3,198.6),0);
  //cp=new Checkpoint(new PVector(100,100),1);
  print("Initial");
  c.debug();
}
void draw(){
  background(bg);

  if (COUNTER < 20){
    c.update(new PVector(275.7,465.9),10);
    ++COUNTER;
    print("Counter-",COUNTER," ");
    c.debug();
  }
    c.show();    
    //cp.show();
}
void mouseClicked(){
  c.debug();
  
}
class Car{
  PVector pos,vel,DirVec;
  float ang_deg; // from +x-axis CW
  int thrust;
  // display vars
  color c;
  float w,h;// a rectangle
  boolean isAngleSet;
  
  Car(PVector ipos,float idir){
    pos=ipos;ang_deg=idir;
    vel=new PVector(0,0);
    thrust=0;
    float ang_rad=radians(ang_deg);
    DirVec=new PVector(cos(ang_rad),sin(ang_rad));
    isAngleSet=false;
    
    c='b';
    w=70;h=40;
  }
  
  void show(){
    pushMatrix();
      translate(pos.x,pos.y);
      rotate(radians(ang_deg));
      fill(255,0,0); noStroke(); rect(-w/2,-h/2,w,h);
      noFill(); stroke(1); line(0,0,w/2+2,0);
    popMatrix();
  }
  
  void setAngle(float iang_deg){
    ang_deg=iang_deg;
  }
  void setPosition(PVector ip){
    pos=ip;
  }
  void update(PVector i_Target,int i_thrust){
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
  
  void update(float i_ang_rad,int i_thrust){
    // car angle is updated using setAngle
    DirVec=new PVector(cos(i_ang_rad),sin(i_ang_rad));
    PVector Acc=DirVec.copy();
    Acc.mult(i_thrust);
    //println("Thrust=",thrust," DirVec=",DirVec," Accel=",Acc);
    vel.add(Acc);
        
    pos.add(vel);
    //println(DirVec,vel,pos);
    
    vel.mult(0.85);
    
    pos.x=truncate(pos.x);pos.y=truncate(pos.y);
    vel.x=truncate(vel.x);vel.y=truncate(vel.y);
    ang_deg=round(degrees(i_ang_rad));
    isAngleSet=true;
  }
  
  PVector getPos(){
    return pos;
  }
  PVector getVel(){
    return vel;
  }
  PVector getAcc(){
    //float ang_rad=radians(ang_deg);
    //PVector DirVec=new PVector(cos(ang_rad),sin(ang_rad));
    PVector Acc=DirVec.copy();
    return Acc.mult(thrust);
  }
  float getAngle(){
    return ang_deg;
  }
  PVector getDirVec(){
    return DirVec;
  }
  void debug(){
    println(" Pos: ",pos," Vel: ",vel,"Ang:",ang_deg);    
  }

};

class Checkpoint{
  PVector pos;
  int num;
  int size=int(600*SCALE);
  int status; // -1 for done, 0 current, 1 for future
  
  Checkpoint(PVector ipos,int inum){
    pos=ipos;
    num=inum;
    status=1;
  }
  void setStatus(int inp){
    status=inp;
  }
  void show(){
    
    if(status==-1){
      fill(150);
    }
    else if(status==0){
      fill(200,200,0);
    }
    else{
      fill(0,200,0);
    }
    noStroke();
    circle(pos.x,pos.y,size);
    fill(0);
    textSize(30);
    text(num,pos.x-10,pos.y+10);
  }
  
};

float truncate(float inp){
  float unscaled_inp=inp/SCALE;
  
  if(unscaled_inp>=0){
    return SCALE*floor(unscaled_inp);
  }
  else{
    return SCALE*ceil(unscaled_inp);
  }
}
