
#include <Wire.h>

enum modeType{ powerControlOnly = 0, constantSpeed = 1, runToPostion = 2};
class HiTechController
{
  private:
    static const uint8_t REG_MANUFACTURER=0x08;
    static const uint8_t REG_VERSION=0x00;
    static const uint8_t REG_SENSOR_TYPE=0x10;
    static const uint8_t NUM_BYTES=8;
    static const uint8_t POWER[2]={0x45,0x46};
    static const uint8_t MODE[2]={0x44,0x47};
    static const uint8_t TARGET[2]={0x40,0x48};
    static const uint8_t CURRENT[2]={0x4C,0x50};
    uint8_t Address;
    bool busy;
  public:
    HiTechController (const uint8_t ID);
    void moveMotors(modeType mode, int power1, int power2, int32_t target1=0, int32_t target2=0);
    //moveMotor(modeType mode, uint8_t power, int32_t target); //TODO
    int32_t getCurrentEncoder(int motorNumber);
    int32_t getTargetEncoder(int motorNumber);
    void resetEncoders();
    void timeOut();
    bool isbusy (int motorNumber);
    uint8_t getModeValue (int motorNumber);
};
HiTechController::HiTechController (const uint8_t ID)
{
  Address = (ID >= 1 && ID <= 4)? ID:1 ;
}
void HiTechController::moveMotors(modeType mode, int power1, int power2, int32_t target1=0, int32_t target2=0)
{
  Wire.beginTransmission(Address);
  Serial.print("begin Transmission to Device : ");Serial.println(Address);
  uint8_t mode1=mode,mode2=mode;byte p1=power1,p2=power2;
  if (power1<0){mode1=mode|8;p1=-1*power1;}
  if (power2<0){mode2=mode|8;p2=-1*power2;}

  if(mode == powerControlOnly || mode == constantSpeed )
  {
    Wire.write(MODE[0]);
    
    Wire.write(mode1);
    Wire.write(p1);
    Wire.write(p2);
    Wire.write(mode2);
  }
  else if(mode == runToPostion)
  {
      Wire.write(TARGET[0]);
      
      uint8_t values1[4];
      for(int i=0; i<4; ++i)
        values1[3-i] = target1 >> i*8;
      
      uint8_t values2[4];
      for(int i=0; i<4; ++i)
        values2[3-i] = target2 >> i*8;
      
      Wire.write(values1, 4);
      Wire.write(mode1);
      Wire.write(p1);
      Wire.write(p2);
      Wire.write(mode2);
      Wire.write(values2, 4);
  }
  Wire.endTransmission(0);
}
int32_t HiTechController::getCurrentEncoder(int motorNumber)
{
  Wire.beginTransmission(Address);
  Wire.write(CURRENT[motorNumber-1]);
  Wire.endTransmission(1);

  int32_t currentEncoder = 0;
  uint8_t i = 0;
  Wire.requestFrom(Address, 4);
  while (Wire.available())
  {
    currentEncoder |= Wire.read();
    ++i;
    if(i<4)
      currentEncoder = currentEncoder<<8;
  }
  return currentEncoder;
}

int32_t HiTechController::getTargetEncoder(int motorNumber)
{
  Wire.beginTransmission(Address);
  Wire.write(TARGET[motorNumber]);
  Wire.endTransmission(1);

  int32_t TargetEncoder = 0;
  uint8_t i = 0;
  Wire.requestFrom(Address, 4);
  while (Wire.available())
  {
    TargetEncoder |= Wire.read();
    ++i;
    if(i<4)
      TargetEncoder = TargetEncoder<<8;
  }
  return TargetEncoder;
}
void HiTechController::resetEncoders()
{
  Wire.beginTransmission(Address);
  Wire.write(MODE[0]);
  Wire.write(3); // reset encoder mode
  Wire.write(0);
  Wire.write(0);
  Wire.write(3);
  Wire.endTransmission(0);
  delay(250);
}
uint8_t HiTechController::getModeValue(int motorNumber)
{
  Wire.beginTransmission(Address); 
  Wire.write(MODE[motorNumber-1]); 
  Wire.endTransmission(1); 
  Wire.requestFrom(Address, 1);
  char ch;
  while (Wire.available())
    ch = Wire.read();
  return ch;
}
bool HiTechController::isbusy(int motorNumber)
{
  busy = getModeValue(motorNumber) >> 7;
  return busy;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
}
HiTechController c1 (0x01);
HiTechController c2 (0x02);
void loop() {
  //Test
  int v0,v1,v2,v3,t0,t1,t2,t3;
  if (Serial.available())
  {
    char Start =Serial.read();
    Serial.println(Start);
    if (Start == ':')
    {
      char type = Serial.read();
      Serial.println(type);
      if (type == 'D')
        {
          char command = Serial.read();
          switch (command)
          {
            case 'd': {
              v0=Serial.parseInt();
              v1=Serial.parseInt();
              v2=Serial.parseInt();
              v3=Serial.parseInt();
              t0=Serial.parseInt();
              t1=Serial.parseInt();
              t2=Serial.parseInt();
              t3=Serial.parseInt();
              c1.moveMotors(runToPostion, v0, v1, t0, t1);
              c2.moveMotors(runToPostion, v2, v3, t2, t3);
              break;}
            case 's':{
              v0=Serial.parseInt();
              v1=Serial.parseInt();
              v2=Serial.parseInt();
              v3=Serial.parseInt();
              c1.moveMotors(constantSpeed, v0, v1);
              c2.moveMotors(constantSpeed, v2, v3);
              break;}
            case 'p': {
              v0=Serial.parseInt();
              Serial.println(v0);
              v1=Serial.parseInt();
              Serial.println(v1);
              v2=Serial.parseInt();
              Serial.println(v2);
              v3=Serial.parseInt();
              Serial.println(v3);
              c1.moveMotors(powerControlOnly, v0, v1);
              c2.moveMotors(powerControlOnly, v2, v3);
              break;}
            case 'r': {
              c1.resetEncoders();
              c2.resetEncoders();
              break; }
          }
        }
    }
  }
}
