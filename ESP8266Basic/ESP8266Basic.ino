//ESP8266 Basic Interperter
//HTTP://ESP8266BASIC.COM
//
//The MIT License (MIT)
//
//Copyright (c) 2015 Michael Molinari
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

//Onewire tempture sensor code conntributed by Rotohammer.

//#include <ArduinoJson.h>
#include "spiffs/spiffs.h"
#include <FS.h>
#include <ESP8266mDNS.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
//#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <ESP8266httpUpdate.h>               // that file need to be copied into the folder for 2.0.0-rc1
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Base64.h"

//#include <WiFiClientSecure.h>
#include "ESP8266httpUpdate.h"
#include <time.h>
//#include <HttpClient.h>                   // that line needs to be commented for esp8266-2.0.0-rc1
//#include <ESP8266HTTPClient.h>              // that line needs to be added for the esp8266-2.0.0 and 2.1.0-rc2

//LCD Stuff
#include <LiquidCrystal_SR.h>
#include <I2CIO.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_SR3W.h>
#include <LCD.h>
#include <LiquidCrystal_SR2W.h>
//#include <FastIO.h>
#include <LiquidCrystal_I2C.h>

//PS2 Key Board
//#include <PS2Keyboard.h>


#include <Time.h>

#include <Adafruit_NeoPixel.h>


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(256, 2, NEO_GRB + NEO_KHZ800);;

//ThingSpeak Stuff


const char BasicVersion[] = "ESP Basic 1.82r10_cicciocb";

#include "expression_parser_string.h"
bool  _parser_failed;
char* _parser_error_msg;
String Line_For_Eval;



OneWire oneWire(2);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address

//PS2Keyboard keyboard;

//WiFiClient EmailClient;
String EmailServer;
int Emailport;
String EmailSMTPuser;
String EmailSMTPpassword;



WiFiClient client;
ESP8266WebServer server(80);

//Web Server Variables
String HTMLout;
PROGMEM const char InputFormText[] = R"=====( <input type="text" id="myid" name="input"><input type="submit" value="Submit" name="inputButton"><hr>)=====";
PROGMEM const char TextBox[] = R"=====( <input type="text" id="myid" name="variablenumber" value="variablevalue">)=====";
PROGMEM const char passwordbox[] = R"=====( <input type="password" id="myid" name="variablenumber" value="variablevalue">)=====";
PROGMEM const char Slider[] = R"=====( <input type="range" id="myid" name="variablenumber" min="minval" max="maxval" value=variablevalue>)=====";
PROGMEM const char GOTObutton[] =  R"=====(<input type="submit" id="myid" value="gotonotext" name="gotonobranch">)=====";
PROGMEM const char GOTOimagebutton[] =  R"=====(<input type="image" id="myid" src="/file?file=gotonotext" value="gotonotext" name="gotonobranch">)=====";
PROGMEM const char normalImage[] =  R"=====(<img src="/file?file=name">)=====";
PROGMEM const char javascript[] =  R"=====(<script src="/file?file=name"></script>)=====";
PROGMEM const char CSSscript[] =  R"=====(<link rel="stylesheet" type="text/css" href="/file?file=name">)=====";
PROGMEM const char DropDownList[] =  R"=====(<select name="variablenumber" id="myid" size="theSize">options</select>
<script>document.getElementsByName("variablenumber")[0].value = "VARS|variablenumber";</script>)=====";
PROGMEM const char DropDownListOpptions[] =  R"=====(<option>item</option>)=====";


String LastElimentIdTag;



byte WaitForTheInterpertersResponse = 1;

PROGMEM const char AdminBarHTML[] = R"=====(
<a href="./vars">[ VARS ]</a> 
<a href="./edit">[ EDIT ]</a>
<a href="./run">[ RUN ]</a>
<a href="./settings">[ SETTINGS ]</a>
<a href="./filemng">[ FILE MANAGER ]</a>
<hr>)=====";



PROGMEM const char UploadPage[] = R"=====(
<form method='POST' action='/filemng' enctype='multipart/form-data'>
<input type='file' name='Upload'>
<input type='submit' value='Upload'>
</form>
<form id="filelist">
<input type="submit" value="Delete" name="Delete">
<input type="submit" value="View" name="View">
<input type="submit" value="" name="View">
</form>

<select name="fileName" size="25" form="filelist">*table*</select>
)=====";



//<a href="http://www.esp8266basic.com/help"  target="_blank">[ HELP ]</a>

PROGMEM const char EditorPageHTML[] =  R"=====(
<script src="editor.js"></script>
<form action='edit' id="usrform">
<input type="text" name="name" value="*program name*">
<input type="submit" value="Open" name="open">
</form>
<button onclick="ShowTheFileList()">Files List</button>
<button onclick="SaveTheCode()">Save</button>
<br>
<textarea rows="30" style="width:100%" name="code" id="code">*program txt*</textarea><br>
<input type="text" id="Status" value="">
)=====";


PROGMEM const char editCodeJavaScript[] =  R"=====(
function SaveTheCode() {
  var textArea = document.getElementById("code");
  var arrayOfLines = textArea.value.split("\n");
  httpGet("/codein?SaveTheCode=start");
  httpGet("/codein?SaveTheCode=yes");
  block = 0;
for (i = 0; i < arrayOfLines.length; i++) 
{ 
  x = i + 1;
  if (arrayOfLines[i] != "undefined")
  {
    arrayOfLines[i] = replaceAll(arrayOfLines[i],"+", "%2B");
    arrayOfLines[i] = replaceAll(arrayOfLines[i],"&", "%26");
    arrayOfLines[i] = replaceAll(arrayOfLines[i],"#", "%23");
  stocca(encodeURI(arrayOfLines[i]));
    document.getElementById("Status").value = i.toString();
  }
}
stocca(">>-save_<<");
document.getElementById("Status").value = "Saved";
  httpGet("/codein?SaveTheCode=end");
alert("Saved");
}
var Sendy="";
var block = 0;
function stocca(s)
{
 Sendy += s;
 block++;
 if ((Sendy.length > 512) || (s == ">>-save_<<"))
 {
  if (s == ">>-save_<<")
   Sendy = Sendy.substr(0,Sendy.length-10); 
   var WhatToSend = "/codein?line=" + block + "&code="+Sendy;
   httpGet(WhatToSend);
   Sendy = "";
 }
 else
 {
   Sendy += "%0D%0A";
 }
}
function ShowTheFileList(){
  var filelist;
  filelist = httpGet("/filelist?all=true");
  alert("List of the Files saved :\n\r" + filelist);
}
function httpGet(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, false ); // false for synchronous request
    xmlHttp.send( null );
    return xmlHttp.responseText;
}
function replaceAll(str, find, replace) {
  for (x = 0; x <= 10; x++) 
  {
    str = str.replace(find, replace);
  }
  return str;
}
)=====";


PROGMEM const char SettingsPageHTML[] =  R"=====(
<form action='settings' id="usrform"><table>
*BasicVersion*
<tr><th>
Station Mode (Connect to your router):</th></tr>
<tr><th><p align="right">Name:</p></th><th><input type="text" name="staName" value="*sta name*"></th></tr>
<tr><th><p align="right">Pass:</p></th><th><input type="password" name="staPass" value="*sta pass*"></th></tr>
<tr><th>
<br><br>Ap mode (ESP brocast out its own ap):</th></tr>
<tr><th><p align="right">Name:</p></th><th><input type="text" name="apName" value="*ap name*"></th></tr>
<tr><th><p align="right">Pass:<br>Must be at least 8 characters</p></th><th><input type="password" name="apPass" value="*ap pass*"></th></tr>
<tr><th>
<br><br>Log In Key (For Security):</th></tr>
<tr><th><p align="right">Log In Key:</p></th><th><input type="password" name="LoginKey" value="*LoginKey*"></th></tr>
<tr><th><p align="right">Display menu bar on index page:</p></th><th><input type="checkbox" name="showMenueBar" value="off" **checked**> Disable<br></th></tr>
<tr><th><p align="right">OTA URL. Leave blank for default:</p></th><th><input type="text" name="otaurl" value="*otaurl*"></th></tr>
<tr><th>
<input type="submit" value="Save" name="save">
<input type="submit" value="Format" name="format">
<input type="submit" value="Update" name="update">
<input type="submit" value="Restart" name="restart">
</th></tr>
</table></form>
<br>
)=====";







PROGMEM const char LogInPage[] =  R"=====(
<form action='settings' id="usrform">
Log In Key
<input type="password" name="key" value="">
<input type="submit" value="login" name="login">
</form>
)=====";


//Graphics HTML CODE

PROGMEM const char GraphicsStartCode[] =  R"=====(<svg width="*wid*" height="*hei*">)=====";

PROGMEM const char GraphicsLineCode[] =  R"=====(<line x1="*x1*" y1="*y1*" x2="*x2*" y2="*y2*" stroke="*collor*"/>)=====";

PROGMEM const char GraphicsCircleCode[] =  R"=====(<circle cx="*x1*" cy="*y1*" r="*x2*" fill="*collor*"/>)=====";

PROGMEM const char GraphicsEllipseCode[] =  R"=====(<ellipse cx="*x1*" cy="*y1*" rx="*x2*" ry="*y2*" fill="*collor*"/>)=====";

PROGMEM const char GraphicsRectangleCode[] =  R"=====(<rect x="*x1*" y="*y1*" width="*x2*" height="*y2*" style="fill:*collor*"/>)=====";



byte numberButtonInUse = 0;
String ButtonsInUse[11];


String   msgbranch;
String   MsgBranchRetrnData;


// Buffer to store incoming commands from serial port
String inData;

int TotalNumberOfLines = 255;
//String BasicProgram[255];                                //Array of strings to hold basic program

String AllMyVaribles[50][2];
int LastVarNumberLookedUp;                                 //Array to hold all of the basic variables
bool VariableLocated;


bool RunningProgram = 1;                                //Will be set to 1 if the program is currently running
byte RunningProgramCurrentLine = 0;                     //Keeps track of the currently running line of code
byte NumberOfReturns;
bool BasicDebuggingOn;
byte ReturnLocations[254];

int TimerWaitTime;
int timerLastActiveTime;
String TimerBranch;

String refreshBranch;

int GraphicsEliments[100][7];

File fsUploadFile;

int noOfLinesForEdit;
String ProgramName = "default";

bool fileOpenFail;


bool inputPromptActive = 0;

int LoggedIn = 0;

int SerialTimeOut;



byte ForNextReturnLocations[255];



Servo Servo0;
Servo Servo1;
Servo Servo2;
Servo Servo3;
Servo Servo4;
Servo Servo5;

Servo Servo12;
Servo Servo13;
Servo Servo14;
Servo Servo15;
Servo Servo16;

String  PinListOfStatus[16];
int  PinListOfStatusValues[16];


//time Stff
int timezone = 3;
int dst = 0;



FSInfo fs_info;

void setup() {
  pixels.begin();
  SPIFFS.begin();
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  WiFi.mode(WIFI_AP_STA);
  PrintAndWebOut(BasicVersion);

  //CheckWaitForRunningCode();



  server.on("/", []()
  {
    String WebOut;
    if (LoadDataFromFile("ShowMenueBar") != "off") WebOut =    AdminBarHTML;
    WebOut += RunningProgramGui();
    server.send(200, "text/html", WebOut);
  });


  server.on("/settings", []()
  {

    server.send(200, "text/html", SettingsPageHandeler());
  });



  server.on("/vars", []()
  {
    String WebOut = AdminBarHTML;
    if ( CheckIfLoggedIn() )
    {
      WebOut = LogInPage;
    }
    else
    {
      WebOut += F("<div style='float: left;'>Variable Dump:");
      for (byte i = 0; i < 50; i++)
      {
        if (AllMyVaribles[i][0] != "" ) WebOut += String("<hr>" + AllMyVaribles[i][0] + " = " + AllMyVaribles[i][1]);
      }


      WebOut += F("<hr></div><div style='float: right;'>Pin Stats");
      for (byte i = 0; i <= 15; i++)
      {
        if ( i < 6 | i > 11) WebOut += String("<hr>" + String(i) + " = " + PinListOfStatus[i] + "  , " + String(PinListOfStatusValues[i]));
      }
      WebOut += "</div>";
    }

    server.send(200, "text/html", WebOut);
  });


  server.on("/run", []()
  {
    String WebOut;
    RunningProgram = 1;
    RunningProgramCurrentLine = 0;
    WaitForTheInterpertersResponse = 0 ;
    numberButtonInUse = 0;
    HTMLout = "";
    TimerWaitTime = 0;
    GraphicsEliments[0][0] = 0;
    WebOut = F(R"=====(  <meta http-equiv="refresh" content="0; url=./input?" />)=====");

    server.send(200, "text/html", WebOut);
  });



  server.onFileUpload(handleFileUpdate);

  server.on("/filemng", []()
  {
    DoSomeFileManagerCode();
  });


  server.on("/edit", []()
  {
    String WebOut;
    if (CheckIfLoggedIn())
    {
      WebOut = String(LogInPage);
      server.send(200, "text/html", String(AdminBarHTML + WebOut ));
      return;
    }
    else
    {
      String CRLF = F("\r\n");
      WaitForTheInterpertersResponse = 1;

      String TextboxProgramBeingEdited;
      //String ProgramName;
      //WebOut = String("<form action='input'>" + HTMLout + "</form>");
      WebOut = String(EditorPageHTML);
      
      if ( server.arg("open") == F("Open") )
      {
        // really takes just the name for the new file otherwise it uses the previous one
        ProgramName = server.arg("name");
        LoadBasicProgramFromFlash(String(F("uploads/"))+ ProgramName + String(F(".bas")));
      }
      // the goal here is to replace the server send function by an equivalent that
      // permit to handle big buffers; this is acheived using the "chunked transfer"
      WebOut = String(EditorPageHTML);
      WebOut = WebOut.substring(0, WebOut.indexOf(F("*program txt*")) );
      WebOut.replace(F("*program name*"), ProgramName);
      
      server.sendContent(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection:close\r\nTransfer-Encoding: chunked\r\nAccess-Control-Allow-Origin: *\r\n\r\n"));
      delay(0);
      // each "chunk" is composed of :
      // the size of the block (in hex) terminated by \r\n
      server.sendContent(String(String(AdminBarHTML).length(), 16) + CRLF); 
      // the block terminated by \r\n
      server.sendContent(String(AdminBarHTML) + CRLF);
      /////// end of first chunk ///////////
      delay(0);
      server.sendContent(String(WebOut.length(), 16) + CRLF);   
      server.sendContent(WebOut + CRLF);
      delay(0);
      int iii;
      int i;
      fileOpenFail = 0;
      for (iii = 1; iii <= TotalNumberOfLines; iii+=50) // important START FROM 1!!!!
      {
        TextboxProgramBeingEdited = "";
        for (i = iii; i < (iii+50); i++)
        {
          delay(0);

          if ( (i > TotalNumberOfLines) || (fileOpenFail == 1) )
    			{
            fileOpenFail = 0;
            iii = 9999;
    				break;
    			}

			  TextboxProgramBeingEdited = TextboxProgramBeingEdited + "\n" + GetRidOfurlCharacters(BasicProgram(i));

        }
        if (TextboxProgramBeingEdited.length() > 0)
        {
          server.sendContent(String(TextboxProgramBeingEdited.length(),16) + CRLF);
          server.sendContent(TextboxProgramBeingEdited + CRLF);
          delay(0);
        }
      }
      Serial.println("avant par ici");
      WebOut = String(EditorPageHTML);
      WebOut = WebOut.substring(WebOut.indexOf(F("</textarea>")));
      server.sendContent(String(WebOut.length(), 16) + CRLF);   
      server.sendContent(WebOut + CRLF);
      // end of transmission
      server.sendContent(F("0\r\n\r\n"));
      server.sendContent(F("0\r\n\r\n"));
      delay(0);
      Serial.println(F("End of Open"));

    }

  });



  server.on("/editor.js", []() {
    server.send(200, "text/html", editCodeJavaScript);
  });


  server.on("/filelist", []() 
  {
    String ret = "";
    String fn;
    Dir dir = SPIFFS.openDir(String(F("uploads/") ));
    while (dir.next()) 
    {
      fn = dir.fileName();  
      if (fn.indexOf(F(".bas")) != -1)
        ret +=  fn +"\n";
      delay(0);
    }
    

    server.send(200, "text/html", ret);
  });

  server.on("/codein", []() {

    if (ProgramName == "")
    {
      ProgramName = F("default");
    }

    if (server.arg("SaveTheCode") == F("start"))
    {
      inData = "end";
      ExicuteTheCurrentLine();
	  Serial.println(F("start save"));
	  OpenToWriteOnFlash(String(F("uploads/")) + ProgramName + String(F(".bas")));
    }

    if (server.arg("SaveTheCode") != F("yes") & server.arg("SaveTheCode") != F("start") & server.arg("SaveTheCode") != F("end"))
    {
      String LineNoForWebEditorIn;
      LineNoForWebEditorIn = server.arg("line");
      int y = LineNoForWebEditorIn.toInt();
      delay(0);
      //Serial.println(server.arg("code"));
	    Serial.println(ProgramName + F(".bas/") + String(y));
      //BasicProgramWriteLine(y, GetRidOfurlCharacters(server.arg("code")));
	    WriteBasicLineOnFlash(GetRidOfurlCharacters(server.arg("code")));
      delay(0);
      noOfLinesForEdit = y;

    }

    if (server.arg("SaveTheCode") == F("end"))
    {
      // terminate the save
      Serial.println(F("end of save!!"));
	    CloseWriteOnFlash();
      LoadBasicProgramFromFlash(String(F("uploads/"))+ ProgramName + String(F(".bas")));
    }
    
    if (server.arg("SaveTheCode") == F("yes"))
    {

//      String directoryToDeleteFilesFrom;
//      directoryToDeleteFilesFrom = String(F(" /data/") + ProgramName;
//      Dir dir1 = SPIFFS.openDir(directoryToDeleteFilesFrom);
//
//      while (dir1.next())
//      {
//        delay(0);
//        File f = dir1.openFile("r");
//        if (dir1.fileName().substring(0, directoryToDeleteFilesFrom.length()) == directoryToDeleteFilesFrom) SPIFFS.remove(dir1.fileName());
//      }
    }
    server.send(200, "text/html", F("good"));
  });





  server.on("/msg", []() {

    MsgBranchRetrnData = F("No MSG Branch Defined");

    if (msgbranch != "")
    {
      inData = String(" goto " + msgbranch + " ");
      WaitForTheInterpertersResponse = 0;
      ExicuteTheCurrentLine();
      runTillWaitPart2();
    }


    server.send(200, "text/html", MsgBranchRetrnData);
  });





  server.on("/input", []() {
    server.send(200, "text/html", RunningProgramGui());
  });

  server.onNotFound ( []() {
    String fileNameToServeUp;
    fileNameToServeUp = GetRidOfurlCharacters(server.arg("file"));
    File mySuperFile = SPIFFS.open(String(F("uploads/")) + fileNameToServeUp, "r");
    if (mySuperFile)
    {
      server.streamFile(mySuperFile, getContentType(fileNameToServeUp));
      //server.send(200, getContentType(fileNameToServeUp), mySuperFile.readString());

    }
    else
    {
      server.send(200, "text/html", RunningProgramGui());
    }
    mySuperFile.close();
  });

  //LoadBasicProgramFromFlash("");


  if (  ConnectToTheWIFI(LoadDataFromFile("WIFIname"), LoadDataFromFile("WIFIpass"), "", "", "") == 0)
  {
    if (LoadDataFromFile("APname") == "")
    {
      CreateAP("", "");
    }
    else
    {
      CreateAP(LoadDataFromFile("APname"), LoadDataFromFile("APpass"));
    }
  }


  Wire.begin(0, 2);

  //  keyboard.begin(14, 12); //For PS2 keyboard input

  StartUp_OLED();
  lcd.begin(16, 2); // initialize the lcd for 16 chars 2 lines and turn on backlight
  sensors.begin();

  LoadBasicProgramFromFlash("uploads/" + ProgramName + ".bas");
  
  server.begin();
  RunningProgram = 0;
  WaitForTheInterpertersResponse = 1;
  StartUpProgramTimer();

}


String SettingsPageHandeler()
{
  if ( server.arg("key") == LoadDataFromFile("LoginKey"))
  {
    LoggedIn = millis();
  }


  WaitForTheInterpertersResponse = 1;
  String WebOut = String(AdminBarHTML) + String(SettingsPageHTML);
  String staName = LoadDataFromFile("WIFIname");
  String staPass = LoadDataFromFile("WIFIpass");
  String apName = LoadDataFromFile("APname");
  String apPass = LoadDataFromFile("APpass");
  String LoginKey = LoadDataFromFile("LoginKey");
  String ShowMenueBar = LoadDataFromFile("ShowMenueBar");
  String otaUrl = LoadDataFromFile("otaUrl");
  //Serial.print("Loading Settings Files");

  if (millis() > LoggedIn + 600000 || LoggedIn == 0 )
  {
    WebOut = LogInPage;
  }
  else
  {

    if ( server.arg("restart") == F("Restart") ) ESP.restart();


    if ( server.arg("update") == F("Update") )
    {

      //        Serial.println(BasicOTAupgrade());
      if (LoadDataFromFile("otaUrl") == "")
      {
        t_httpUpdate_return  ret = ESPhttpUpdate.update(F("esp8266basic.smbisoft.com"), 80, F("/4M/ESP8266Basic.cpp.bin"));
        if (ret == HTTP_UPDATE_FAILED ) Serial.println(F("Update failed"));
      }
      else
      {
        String URLtoGet = LoadDataFromFile("otaUrl");
        String ServerToConnectTo;
        String PageToGet;
        ServerToConnectTo = URLtoGet.substring(0, URLtoGet.indexOf("/"));
        PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));
        t_httpUpdate_return  ret = ESPhttpUpdate.update(ServerToConnectTo, 80, PageToGet);
        if (ret == HTTP_UPDATE_FAILED ) Serial.println(F("Update failed"));
      }
      //t_httpUpdate_return  ret = ESPhttpUpdate.update("cdn.rawgit.com", 80, "/esp8266/Basic/master/Flasher/Build/4M/ESP8266Basic.cpp.bin");

    }


    if ( server.arg("save") == F("Save") )
    {
      staName = GetRidOfurlCharacters(server.arg("staName"));
      staPass = GetRidOfurlCharacters(server.arg("staPass"));
      apName  = GetRidOfurlCharacters(server.arg("apName"));
      apPass  = GetRidOfurlCharacters(server.arg("apPass"));
      LoginKey = GetRidOfurlCharacters(server.arg("LoginKey"));
      ShowMenueBar = GetRidOfurlCharacters(server.arg("showMenueBar"));
      otaUrl       = GetRidOfurlCharacters(server.arg("otaurl"));

      SaveDataToFile("WIFIname" , staName);
      SaveDataToFile("WIFIpass" , staPass);
      SaveDataToFile("APname" , apName);
      SaveDataToFile("APpass" , apPass);
      SaveDataToFile("LoginKey" , LoginKey);
      SaveDataToFile("ShowMenueBar" , ShowMenueBar);
      SaveDataToFile("otaUrl" , otaUrl);
    }

    if ( server.arg("format") == F("Format") )
    {
      Serial.println(F("Formating "));
      Serial.print(SPIFFS.format());
    }

    WebOut.replace(F("*sta name*"), staName);
    WebOut.replace(F("*sta pass*"), staPass);
    WebOut.replace(F("*ap name*"),  apName);
    WebOut.replace(F("*ap pass*"),  apPass);
    WebOut.replace(F("*LoginKey*"), LoginKey);
    WebOut.replace(F("*BasicVersion*"), BasicVersion);
    WebOut.replace(F("*otaurl*"), otaUrl);

    if ( ShowMenueBar == F("off"))
    {
      WebOut.replace(F("**checked**"), F("checked"));
    }
    else
    {
      WebOut.replace(F("**checked**"), "");
    }
  }
return WebOut;
}



String getContentType(String filename) {
  if (filename.endsWith(".htm")) return F("text/html");
  else if (filename.endsWith(".html")) return F("text/html");
  else if (filename.endsWith(".htm")) return F("text/html");
  else if (filename.endsWith(".css")) return F("text/css");
  else if (filename.endsWith(".js")) return F("application/javascript");
  else if (filename.endsWith(".png")) return F("image/png");
  else if (filename.endsWith(".gif")) return F("image/gif");
  else if (filename.endsWith(".jpg")) return F("image/jpeg");
  else if (filename.endsWith(".ico")) return F("image/x-icon");
  else if (filename.endsWith(".xml")) return F("text/xml");
  else if (filename.endsWith(".pdf")) return F("application/x-pdf");
  else if (filename.endsWith(".zip")) return F("application/x-zip");
  else if (filename.endsWith(".gz")) return F("application/x-gzip");
  return "text/plain";
}

void StartUpProgramTimer()
{
  while  (millis() < 60000)
  {
    delay(0);
    //Serial.println(millis());
    server.handleClient();
    if (WaitForTheInterpertersResponse == 0) return;
  }
  Serial.println(F("Starting Default Program"));
  RunningProgram = 1;
  RunningProgramCurrentLine = 0;
  WaitForTheInterpertersResponse = 0 ;
  numberButtonInUse = 0;
  HTMLout = "";
  return;
}



void DoSomeFileManagerCode()
{
  String WholeUploadPage = UploadPage;
  String FileListForPage ;

  if (CheckIfLoggedIn())
  {
    WholeUploadPage = LogInPage;
  }
  else
  {
    if (server.arg("Delete") != "")
    {
      String FIleNameForDelete = server.arg("fileName");
      FIleNameForDelete = GetRidOfurlCharacters(FIleNameForDelete);
      Serial.println(FIleNameForDelete);
      SPIFFS.remove(FIleNameForDelete);
      //Serial.println(SPIFFS.remove("uploads/settings.png"));
    }

    Dir dir = SPIFFS.openDir(String(F("uploads") ));
    while (dir.next()) {
      FileListForPage += String(F("<option>")) + dir.fileName() + String(F("</option>"));
      delay(0);
    }

    WholeUploadPage.replace("*table*", FileListForPage);

    if (server.arg("View") != "")
    {
      String FileNameToView = server.arg("fileName");
      FileNameToView = GetRidOfurlCharacters(FileNameToView);
      FileNameToView.replace("uploads/", "");
      WholeUploadPage = F(R"=====(  <meta http-equiv="refresh" content="0; url=./file?file=item" />)=====");
      WholeUploadPage.replace("item", FileNameToView);
    }

  }
  server.send(200, "text/html",  String( AdminBarHTML + WholeUploadPage ));
}



void handleFileUpdate()
{
  //if (server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    //DBG_OUTPUT_PORT.print("Upload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(String("uploads/" + filename), "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("Upload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    //DBG_OUTPUT_PORT.print("Upload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}








String  getSerialInput()
{
  unsigned long  serialTimeOutStart = millis();
  bool donereceivinginfo = 0;
  Serial.println(">");

  String someInput;
  while (donereceivinginfo == 0)
  {
    if (serialTimeOutStart + SerialTimeOut < millis() & SerialTimeOut != 0) return someInput;
    delay(0);
    while (Serial.available() > 0)
    {
      char recieved = Serial.read();
      // Process message when new line character is recieved
      if (recieved == '\n')
      {
        Serial.println(someInput);
        donereceivinginfo = 1;
        return someInput;
      }
      someInput += recieved;
    }
  }
}






//String GetPS2input()
//{
//  String PS2inputString;
//
//
//  bool donereceivinginfo = 0;
//
//  while (donereceivinginfo == 0)
//  {
//    delay(0);
//    while (keyboard.available())
//    {
//      delay(0);
//      // read the next key
//      char c = keyboard.read();
//
//      // check for some of the special keys
//      if (c == PS2_ENTER) {
//        return PS2inputString;
//      } else if (c == PS2_TAB) {
//        //Serial.print("[Tab]");
//      } else if (c == PS2_ESC) {
//        return "";
//      } else if (c == PS2_PAGEDOWN) {
//        //Serial.print("[PgDn]");
//      } else if (c == PS2_PAGEUP) {
//        //Serial.print("[PgUp]");
//      } else if (c == PS2_LEFTARROW) {
//        //Serial.print("[Left]");
//      } else if (c == PS2_RIGHTARROW) {
//        //Serial.print("[Right]");
//      } else if (c == PS2_UPARROW) {
//        //Serial.print("[Up]");
//      } else if (c == PS2_DOWNARROW) {
//        //Serial.print("[Down]");
//      } else if (c == PS2_DELETE) {
//        //Serial.print("[Del]");
//      } else {
//        if (BasicDebuggingOn == 1) Serial.print(c);
//        PS2inputString += c;
//      }
//    }
//  }
//}






bool CheckIfLoggedIn()
{
  if (LoadDataFromFile("LoginKey") != "")
  {
    if ( millis() > LoggedIn + 600000 || LoggedIn == 0 )     return 1;
  }
  return 0;
}










void loop()
{

  RunBasicTillWait();
  delay(0);
  server.handleClient();
}



void RunBasicTillWait()
{
  runTillWaitPart2();
  if (RunningProgramCurrentLine > TotalNumberOfLines)
  {
    RunningProgram = 0 ;
    TimerWaitTime = 0;
    return;
  }

  if (TimerWaitTime + timerLastActiveTime <= millis() &  TimerWaitTime != 0)
  {
    inData = String(" goto " + TimerBranch + " ");
    WaitForTheInterpertersResponse = 0;
    timerLastActiveTime = millis() ;
    ExicuteTheCurrentLine();
    runTillWaitPart2();
  }
  delay(0);
  for (int pinnn = 0; pinnn <= 15 ; pinnn++)
  {
    delay(0);
    //Serial.println(pinnn);
    if ((PinListOfStatus[pinnn] != "po") & ( PinListOfStatus[pinnn] != "pi") & (PinListOfStatus[pinnn] != "pwi") & (PinListOfStatus[pinnn] != "pwo") & (PinListOfStatus[pinnn] != "servo") & ( PinListOfStatus[pinnn] != ""))
    {
      //Serial.println("Foud interupt pin");
      if ( PinListOfStatusValues[pinnn] != UniversalPinIO("pi", String(pinnn), 0))
      {
        inData = String(" goto " + PinListOfStatus[pinnn] + " ");
        WaitForTheInterpertersResponse = 0;
        //Serial.println(PinListOfStatus[pinnn]);
        ExicuteTheCurrentLine();
        runTillWaitPart2();
      }
    }
  }
}


void runTillWaitPart2()
{
  while (RunningProgram == 1 && RunningProgramCurrentLine < TotalNumberOfLines && WaitForTheInterpertersResponse == 0 )
  {
    delay(0);
    RunningProgramCurrentLine++;
    inData = BasicProgram(RunningProgramCurrentLine);
    if (fileOpenFail == 1) inData  = "end";
    ExicuteTheCurrentLine();
    delay(0);
  }
}





String getValueforPrograming(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}




String getValue(String data, char separator, int index)
{
  data = String(data + "           ");
  int maxIndex = data.length() - 1;
  int j = 0;
  byte WaitingForQuote;
  String chunkVal = "";
  String ChunkReturnVal;
  for (int i = 0; i <= maxIndex && j <= index; i++)
  {
    if (data[i] == '\"' )
    {
      i++;
      while (i <= maxIndex && data[i] != '\"' ) {
        chunkVal.concat(data[i]);
        i++;
        delay(0);
      }
    }
    else if (data[i] == '|' )
    {
      i++;
      while (i <= maxIndex && data[i] != '|' ) {
        chunkVal.concat(data[i]);
        i++;
        delay(0);
      }
    }
    else
    {
      if (data[i] != separator) chunkVal.concat(data[i]);
    }

    if (data[i] == separator & data[i - 1] != separator)
    {
      j++;
      if (j > index)
      {
        //chunkVal.trim();
        if (chunkVal != String(separator))
        {
          ChunkReturnVal = chunkVal;
          break;
        }
      }
      chunkVal = "";
    }
  }
  //    Serial.println("index");
  //    Serial.println(index);
  //    Serial.println(j);

  if (j == index + 1)
  {

    return ChunkReturnVal;
  }
}












String DoMathForMe(String cc, String f, String dd )
{
  double e;
  String ee = cc;

  double c = cc.toFloat();
  double d = dd.toFloat();

  f.trim();

  if (f == "-") {
    e = c - d;
    ee = String(e);
  }
  if (f == "+") {
    e = c + d;
    ee = String(e);
  }
  if (f == "*") {
    e = c * d;
    ee = String(e);
  }
  if (f == "/") {
    e = c / d;
    ee = String(e);
  }
  if (f == "^") {
    e = pow(c , d);
    ee = String(e);
  }



  if (f == "&") {
    ee = String(cc + dd);
  }


  if (f ==  ">") {
    ee = String((c > d));
  }
  if (f ==  "<") {
    ee = String((c < d));
  }


  if (f ==  ">=") {
    ee = String((c >= d));
  }
  if (f ==  "<=") {
    ee = String((c <= d));
  }

  if (f == "<>" || f == " != ")
  {
    if (cc != dd)
    {
      ee = "1";
    }
    else
    {
      if (c != d)
      {
        ee = "1";
      }
    }
  }


  if (f == "==")
  {
    ee = String((cc == dd));
  }

  if (f == "=")
  {
    ee = String((c == d));
  }
  return ee;
}



String FetchOpenWeatherMapApi(String URLtoGet, String index)
{
  String ServerToConnectTo = URLtoGet.substring(0, URLtoGet.indexOf("/"));
  String PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));

  String s = "";
  char c;
  int cnt = 0;
  int phase = 0;
  int graffe = 0;

  const char lookforLIST[] = "\"list\":[";
  const char lookforBEGIN[] = "{\"";
  char ptr = 0;

  int list = index.toInt();

  if (list == 0)  // if the index is 0, it takes the first part, the root
    phase = 3; 
    
  if (client.connect(ServerToConnectTo.c_str() , 80))
  {
    client.print(String("GET " + PageToGet + " HTTP/1.1\r\nHost: " +  ServerToConnectTo + "\r\n\r\n"));
    delay(300);

    while (client.available())
    {
        delay(0);
        //delay(1);
        c = client.read();
        delay(0);
        //Serial.print(c);
        switch (phase)
        {
          case 0:
            if (c == lookforLIST[ptr])
                ptr++;
            else
                ptr = 0;
            
            if (ptr == strlen(lookforLIST))
            {
                phase = 1;
                list = list -1;
                if (list == 0)
                {
                  phase = 2;
                 }
                //Serial.println("phase 0 OK");
            }
            break;
            
          case 1:
            if (c == '{')
              graffe++;
            else
              if (c == '}')
                graffe--;

            if (graffe == 0) 
            {
                if ( (c == ',') || (c == ']') )
                    list--;
            }
            if (list == 0)
            {
              phase = 2;
              //Serial.println("phase 1 OK");
            }
            break;            

          case 2:
            s.concat(c);
            cnt++;
            if (c == '{')
              graffe++;
            else
              if (c == '}')
                graffe--;

            if ((graffe == 0) || (cnt > 600)) // max 600 chars
            {
                //Serial.println("phase 2 OK");
                client.stop();
                return s;
            }
              
            break;

          case 3:    // search the beginning of the message starting with  {"
            if (c == lookforBEGIN[ptr])
                ptr++;
            else
                ptr = 0;
            
            if (ptr == strlen(lookforBEGIN))
            {
                cnt = 2;
                phase = 4;
                //Serial.println("phase 3 OK");
                s = F("{\"");
            }
            break;

          case 4:
            s.concat(c);
            cnt++;
            if (c == lookforLIST[ptr])
                ptr++;
            else
                ptr = 0;
            
            if ( (ptr == strlen(lookforLIST)) || (cnt > 1000)) // max 1000 chars
            {
                //Serial.println("phase 4 OK");
                client.stop();
                s.concat("]}");
                return s;
            }
            break;            
        }

    if (client.available() == false)
    {
      // if no data, wait for 300ms hoping that new data arrive
      delay(300);
    }
      
    }//while
    client.stop();
    return F("END OF DATA REACHED");

  }
  client.stop();
  return "";
}








String FetchWebUrl(String URLtoGet)
{
  String str = "             ";
  String ServerToConnectTo = URLtoGet.substring(0, URLtoGet.indexOf("/"));
  String PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));
  // ServerToConnectTo ;
  //PageToGet = URLtoGet.substring(URLtoGet.indexOf("/"));

//  Serial.println(ServerToConnectTo);
//  Serial.println(PageToGet);


  if (client.connect(ServerToConnectTo.c_str() , 80))
  {
    client.print(String("GET " + PageToGet + " HTTP/1.1\r\nHost: " +  ServerToConnectTo + "\r\n\r\n"));
    delay(300);
    while (client.available())
    {
      delay(0);
      if (str.endsWith(String("\r\n\r\n")))  str = "";

      str.concat( (const char)client.read());
      delay(0);
    }



    client.stop();
    return str.substring(0, str.indexOf(String(String(char(10)) + "0" )  ));
  }
  client.stop();
  return "";
}



void serialFlush()
{

  while (Serial.available() > 0)
  {
    delay(0);
    char t = Serial.read();
  }
}



