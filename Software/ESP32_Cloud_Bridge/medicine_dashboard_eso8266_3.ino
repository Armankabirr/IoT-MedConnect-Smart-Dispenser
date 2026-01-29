#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ---------- WiFi ----------
const char* ssid = "Razib Pc Net";
const char* password = "1234567890";

// ---------- Web Server ----------
ESP8266WebServer server(80);

// ---------- EEPROM addresses ----------
#define ADDR_MORNING_H 0
#define ADDR_MORNING_M 1
#define ADDR_NOON_H 2
#define ADDR_NOON_M 3
#define ADDR_NIGHT_H 4
#define ADDR_NIGHT_M 5

// ---------- Pill times ----------
int MORNING_HOUR, MORNING_MIN;
int NOON_HOUR, NOON_MIN;
int NIGHT_HOUR, NIGHT_MIN;

// ---------- NTP Client ----------
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 6*3600, 60000); // GMT+6, updates every 1 min

// ---------- Nano Status ----------
bool mTaken = false, nTaken = false, eTaken = false;

// =================================================
// Format time helper
String formatTime(int h,int m){
  return (h<10?"0":"")+String(h)+":"+(m<10?"0":"")+String(m);
}

// =================================================
// Send times to Arduino Nano
void sendTimesToNano(){
  String msg = "TIME:M="+formatTime(MORNING_HOUR,MORNING_MIN)+
               ";N="+formatTime(NOON_HOUR,NOON_MIN)+
               ";E="+formatTime(NIGHT_HOUR,NIGHT_MIN)+";";
  Serial.println(msg);
}

// =================================================
// Calculate remaining next pill time
String remainingTime(int h, int m, int s){
  int nh,nm;
  if(h<MORNING_HOUR || (h==MORNING_HOUR && m<MORNING_MIN)){ nh=MORNING_HOUR; nm=MORNING_MIN;}
  else if(h<NOON_HOUR || (h==NOON_HOUR && m<NOON_MIN)){ nh=NOON_HOUR; nm=NOON_MIN;}
  else if(h<NIGHT_HOUR || (h==NIGHT_HOUR && m<NIGHT_MIN)){ nh=NIGHT_HOUR; nm=NIGHT_MIN;}
  else{ nh=MORNING_HOUR; nm=MORNING_MIN;} // next day

  long now = h*3600L + m*60L + s;
  long next = nh*3600L + nm*60L;
  if(next<=now) next+=86400; // add 24h if past

  long diff = next - now;
  char buf[20];
  sprintf(buf,"%ldh %ldm %lds", diff/3600, (diff%3600)/60, diff%60);
  return String(buf);
}

// =================================================
// Dashboard HTML (unchanged)
String dashboardHTML(){
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Medicine Dashboard</title>
<style>
body{font-family:Arial;background:#eef2f5;margin:0;padding:0;}
.container{width:90%;margin:20px auto;}
.card{background:#fff;margin:15px;padding:20px;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,.15);}
h1{text-align:center;margin:5px 0;color:white;}
.header{background:#1976d2;padding:15px;border-radius:12px 12px 0 0;}
h2{text-align:center;margin:5px 0;color:#1976d2;}
.time{font-size:28px;text-align:center;font-weight:bold;color:#444;}
.status{text-align:center;font-size:18px;margin-top:8px;color:#555;}
.time-row{display:flex;align-items:center;margin:8px 0;}
.time-row label{width:25%;text-align:right;margin-right:10px;font-weight:bold;color:#1976d2;}
.time-row input{width:50%;padding:8px;font-size:16px;border-radius:6px;border:1px solid #ccc;}
.led{width:20px;height:20px;border-radius:50%;display:inline-block;margin-left:10px;}
button{width:100%;padding:12px;font-size:17px;background:#1976d2;color:white;border:none;border-radius:8px;cursor:pointer;}
button:hover{background:#125aa6;}
</style>
</head>
<body>
<div class="container">

<!-- Header -->
<div class="card header">
  <h1>Medicine Schedule Monitor</h1>
</div>

<div class="card">
  <h2>Live Time</h2>
  <div class="time" id="t">--:--:--</div>
</div>

<div class="card">
  <h2>Medicine Schedule</h2>
  <div class="time-row"><label>Morning</label><input type="time" id="m"><div id="m_led" class="led" style="background:red"></div></div>
  <div class="time-row"><label>Noon</label><input type="time" id="n"><div id="n_led" class="led" style="background:red"></div></div>
  <div class="time-row"><label>Night</label><input type="time" id="e"><div id="e_led" class="led" style="background:red"></div></div>
  <button onclick="save()">Save Schedule</button>
</div>

<div class="card">
  <h2>Next Pill Status</h2>
  <div class="status" id="r">--</div>
</div>

</div>

<script>
let initialized=false;

function load(){
  fetch('/data').then(r=>r.json()).then(d=>{
    document.getElementById('t').innerHTML=d.time;
    document.getElementById('r').innerHTML=d.remain;
    if(!initialized){
      document.getElementById('m').value=d.m;
      document.getElementById('n').value=d.n;
      document.getElementById('e').value=d.e;
      initialized=true;
    }
    document.getElementById('m_led').style.background = d.m_taken ? 'green':'red';
    document.getElementById('n_led').style.background = d.n_taken ? 'green':'red';
    document.getElementById('e_led').style.background = d.e_taken ? 'green':'red';
  });
}

function save(){
  fetch(`/set?m=${document.getElementById('m').value}&n=${document.getElementById('n').value}&e=${document.getElementById('e').value}`);
  alert("Schedule Saved!");
}

setInterval(load,1000);
</script>
</body>
</html>
)rawliteral";
}

// =================================================
// Setup
void setup(){
  Serial.begin(9600);
  EEPROM.begin(512);

  // Load times from EEPROM
  MORNING_HOUR = EEPROM.read(ADDR_MORNING_H);
  MORNING_MIN  = EEPROM.read(ADDR_MORNING_M);
  NOON_HOUR    = EEPROM.read(ADDR_NOON_H);
  NOON_MIN     = EEPROM.read(ADDR_NOON_M);
  NIGHT_HOUR   = EEPROM.read(ADDR_NIGHT_H);
  NIGHT_MIN    = EEPROM.read(ADDR_NIGHT_M);

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){delay(500);}

  timeClient.begin();
  timeClient.update();

  // Routes
  server.on("/", [](){ server.send(200,"text/html",dashboardHTML()); });

  server.on("/data", [](){
    timeClient.update();
    int h = timeClient.getHours();
    int m = timeClient.getMinutes();
    int s = timeClient.getSeconds();

    String json="{";
    char buf[10];
    sprintf(buf,"%02d:%02d:%02d",h,m,s);
    json += "\"time\":\""+String(buf)+"\",";
    json += "\"remain\":\""+remainingTime(h,m,s)+"\",";
    json += "\"m\":\""+formatTime(MORNING_HOUR,MORNING_MIN)+"\",";
    json += "\"n\":\""+formatTime(NOON_HOUR,NOON_MIN)+"\",";
    json += "\"e\":\""+formatTime(NIGHT_HOUR,NIGHT_MIN)+"\",";
    json += "\"m_taken\":"+(String(mTaken?"true":"false"))+",";
    json += "\"n_taken\":"+(String(nTaken?"true":"false"))+",";
    json += "\"e_taken\":"+(String(eTaken?"true":"false"));
    json += "}";
    server.send(200,"application/json",json);
  });

  server.on("/set", [](){
    if(server.hasArg("m")){
      String t=server.arg("m");
      MORNING_HOUR=t.substring(0,2).toInt();
      MORNING_MIN=t.substring(3,5).toInt();
      EEPROM.write(ADDR_MORNING_H,MORNING_HOUR);
      EEPROM.write(ADDR_MORNING_M,MORNING_MIN);
    }
    if(server.hasArg("n")){
      String t=server.arg("n");
      NOON_HOUR=t.substring(0,2).toInt();
      NOON_MIN=t.substring(3,5).toInt();
      EEPROM.write(ADDR_NOON_H,NOON_HOUR);
      EEPROM.write(ADDR_NOON_M,NOON_MIN);
    }
    if(server.hasArg("e")){
      String t=server.arg("e");
      NIGHT_HOUR=t.substring(0,2).toInt();
      NIGHT_MIN=t.substring(3,5).toInt();
      EEPROM.write(ADDR_NIGHT_H,NIGHT_HOUR);
      EEPROM.write(ADDR_NIGHT_M,NIGHT_MIN);
    }
    EEPROM.commit();
    sendTimesToNano();
    server.send(200,"text/plain","Saved");
  });

  server.begin();
}

// =================================================
// Loop
void loop(){
  server.handleClient();

  // ---------- FIXED Nano STATUS PARSER ----------
  while(Serial.available()){
    String line = Serial.readStringUntil('\n');
    line.trim();
    if(line.startsWith("STATUS:")){
      // Example: STATUS:MORNING=1,NOON=0,NIGHT=1
      int mi=line.indexOf("MORNING="); 
      int ni=line.indexOf("NOON="); 
      int ei=line.indexOf("NIGHT=");

      if(mi!=-1) mTaken=line.substring(mi+8, mi+9).toInt();
      if(ni!=-1) nTaken=line.substring(ni+5, ni+6).toInt();
      if(ei!=-1) eTaken=line.substring(ei+6, ei+7).toInt();
    }
  }
}
