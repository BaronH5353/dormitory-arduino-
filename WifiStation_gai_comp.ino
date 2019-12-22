int RECV_PIN = 10; //D10接红外接收传感器
#include <IRremote.h>
#include <avr/pgmspace.h>
//#define buzzer_pin 8 //定义蜂鸣器驱动引脚
#define PIR_PIN 6//rentihongwai
//#define buzzer_fre 600 //定义蜂鸣器输出频率
//int buttonState; int cond=0;
IRrecv irrecv(RECV_PIN);                        //初始化红外该接收解码对象
decode_results results;                        //初始化解码结果对象
IRsend irsend; 
const unsigned int on[200]PROGMEM = 
{4397,4368,
561,1578,560,537,557,1581,584,1582,584,486,558,537,584,1554,560,538,
558,510,558,1607,557,513,585,511,556,1585,581,1583,583,486,557,1609,
556,513,558,538,557,512,558,1607,558,1583,557,1608,558,1608,557,1580,
559,1606,559,1581,556,1609,557,512,559,537,558,512,557,539,557,512,
558,1607,558,1582,558,537,584,1555,558,1607,556,514,559,537,556,513,
558,537,556,514,558,1606,559,511,583,513,558,1582,557,1608,558,1580,
585,5154,4419,4347,584,1555,558,539,557,1581,584,1581,571,501,577,516,
571,1568,557,541,581,487,557,1608,557,513,558,538,557,1582,583,1582,584,
487,557,1608,557,511,611,486,558,510,558,1607,557,1609,556,1583,583,1582,
583,1555,558,1607,558,1607,559,1582,557,537,585,486,558,510,558,540,558,
509,557,1608,558,1608,557,512,559,1606,559,1581,558,537,581,490,558,510,
587,511,557,511,559,1606,559,510,569,527,558,1582,560,1604,561,1605,561};
#define SSID        "why" //改为你的热点名称, 不要有中文
#define PASSWORD    "qgdsb123"//改为你的WiFi密码Wi-Fi密码
#define DEVICEID    "561953574" //OneNet上的设备ID
String apiKey = "jbS6UIjyvXQW2fkez2GpPBp15eE=";//与你的设备绑定的APIKey

/***/
#define HOST_NAME   "api.heclouds.com"
#define HOST_PORT   (80)
#define INTERVAL_SENSOR   5000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      5000             //定义发送时间
//传感器部分================================   
#include <Wire.h>                                  //调用库  
#include <ESP8266.h>
#include <I2Cdev.h>                                //调用库  
/*******温湿度*******/
#include <Microduino_SHT2x.h>
/*******光照*******/
//#define  sensorPin_1  A0
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

//WEBSITE     
char buf[10];

#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();

//#define INTERVAL_OLED 1000

//String mCottenData;
String jsonToSend;

//3,传感器值的设置 
float sensor_tem, sensor_hum, sensor_lux;                    //传感器温度、湿度、光照   
char  sensor_tem_c[7], sensor_hum_c[7], sensor_lux_c[7];    //换成char数组传输
#include <SoftwareSerial.h>
#define EspSerial mySerial
#define UARTSPEED  9600
SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(&EspSerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

Tem_Hum_S2 TempMonitor;

void setup(void)     //初始化函数  
{       
  //初始化串口波特率  
    Wire.begin();
    Serial.begin(38400);
    irrecv.enableIRIn(); // 启动红外解码
    WifiInit(EspSerial, UARTSPEED);
    pinMode(PIR_PIN, INPUT);   //PIR传感器接口
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    //Serial.print(F("setup begin\r\n"));
    delay(100);
    //pinMode(sensorPin_1, INPUT);
  
  //Serial.print(F("FW Version:"));
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print(F("ok\r\n"));
  } else {
    Serial.print(F("err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("suc\r\n"));

    //Serial.print(F("IP:"));
    //Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("APfail\r\n"));
  }

  /*if (wifi.disableMUX()) {
    Serial.print(F("single ok\r\n"));
  } else {
    Serial.print(F("single err\r\n"));
  }*/

  Serial.print(F("set end\r\n"));
    
  
}
void loop(void)     //循环函数  
{   
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  }
  if(sensor_tem<10){irsend.sendRaw(pgm_read_byte(&on), 200, 38);}
  delay(3000);
}

void getSensorData(){  
    sensor_tem = TempMonitor.getTemperature();  
    sensor_hum = TempMonitor.getHumidity();   
    //获取光照
    //sensor_lux = analogRead(A0);    
   if (digitalRead(PIR_PIN))  //如果传感器返回high
  {delay(100); sensor_lux=1;} else {sensor_lux=0;}   
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
    dtostrf(sensor_lux, 3, 1, sensor_lux_c);
    //dtostrf(sensor_al, 1, 2, sensor_al_c);
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print(F("tcpok\r\n"));

jsonToSend="{\"Temp\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Hum\":";
    dtostrf(sensor_hum,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Lux\":";
    dtostrf(sensor_lux,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+="}";

    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println(F("send"));   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        //Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print( F("release err\r\n"));
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print(F("tcperr\r\n"));
  }
}
