// Close switch must be connected to D2
// D1 is the output for the garade door motor. Hi-low-Hi

//On line 18 you must enter your ssid
//On line 19 you must enter your router password
//For now the password is Tru$tNo01. This password must be in you internet shortcut. If you want to change this, hit ctrl-f enter Tru$tNo01, your new password and hit replace all.

#include <ESP8266WiFi.h>


const int statusPin = 4; //pin for the magnetic contact switch/ Open/Close D2
const int activatePin = 5; //pin for the relay module D1


WiFiServer server(80); //This is the port to listen. Need to be in your router to open the door remotely. If you need only local. Put 80 instead and forget the router part
WiFiClient client;

const char* SSID = "631Sherwood";
const char* PASS = "jbermine1";


void setup() {
  Serial.begin(115200);
  pinMode(statusPin, INPUT_PULLUP);
  pinMode(activatePin, OUTPUT);
  digitalWrite(activatePin, HIGH);
  delay(50);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  
  //Start the server
  server.begin();
  Serial.println("Server started");

  //Print the IP Address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(500);
}

void loop() 
{
  client = server.available();
  if (client)  // if you get a client
  {
    char getLine[128];
    int i = 0;
    bool getLineFound = false;
    bool currentLineIsBlank = true;

    Serial.println("");
    Serial.println("new client");

    while (client.connected()) // loop while the client's connected
    {
      if (client.available()) // if there's bytes to read from the client
      {
        char c = client.read(); // read 1 byte from client
        Serial.print(c);

        if (!getLineFound && i < sizeof(getLine))
        {
          //save the char to getLine
          getLine[i] = c;
          i++;
        }
        
         // Request end: Now responds to it
        if (c == '\n' && currentLineIsBlank) // respond to client only after last line is received, last line is blank and ends with \n
        {
          ProcessRequest(getLine);
          break;
        }

        if (c == '\n') // end of the line, next char read will be a new line
        {
          if (!getLineFound) //the GET line is the first line from the client, save it for later
          {
            getLineFound = true;

            //strip off the HTTP/1.1 from the end of the getLine
            const char *ptr = strstr(getLine, "HTTP/1.1");
            if (ptr)
              getLine[ptr - getLine - 1] = '\0';
          }

          currentLineIsBlank = true;
        }
        else if (c != '\r') //text char received
        {
          currentLineIsBlank = false;
        }
      } //end if (client.available())
    } //end while (client.connected())


    // close the connection
    delay(100); //allow client to receive the data
    client.stop();
    Serial.println("Client disconnected");
  } 
}


void mainPage(WiFiClient& client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: keep-alive\r\n");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Garage Door Monitor</title>");
  client.println("<script>");
  client.println("function GetSwitchState() {");
  client.println("nocache = \"&nocache=\"+ Math.random() * 1000000;");
  client.println("var request = new XMLHttpRequest();");
  client.println("request.onreadystatechange = function() {");
  client.println("if (this.readyState == 4) {");
  client.println("if (this.status == 200) {");
  client.println("if (this.responseText != null) {");
  client.println("document.getElementById(\"switch_txt\").innerHTML = this.responseText;");
  client.println("}}}}");
  client.println("request.open(\"GET\", \"ajaxswitch\" + nocache, true);");
  client.println("request.send(null);");
  client.println("setTimeout('GetSwitchState()', 1000);");
  client.println("}");
  client.println("</script>");
  client.println("</head>");
  client.println("<body onload=\"GetSwitchState()\">");
  client.println("<center>");
  client.println("<h1>Garage Door Monitor</h1>");
  client.println("<hr>\r\n<br>");
  client.println("<button style=\"width:100%;font-size: 50px;\"><a href=\"/Tru$tNo01/activate\">Activate</a></button>");
  client.println("<p id=\"switch_txt\">Garage Door is now:</p>");
  client.println("</center>\r\n</body>\r\n</html>");

}


void redirect(WiFiClient& client)
{
  client.println("HTTP/1.1 303 See other");
  client.println("Location: /Tru$tNo01");
}

void ActivateDoor()
{
  digitalWrite(activatePin, LOW);
  delay(500);
  digitalWrite(activatePin, HIGH);
}

void doorState(WiFiClient& client){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: keep-alive\r\n"); 
  
  if (digitalRead(statusPin) == 1) {
    client.println("Garage Door is now: <font color=\"red\">Open</font>");
  }
  else {
    client.println("Garage Door is now: <font color=\"green\">Close</font>");
  }
}


void ProcessRequest(char* getLine)
{
  
  if (strstr(getLine, "GET /Tru$tNo01") != NULL)
  {
    if (strstr(getLine, "GET /Tru$tNo01/activate") != NULL)
    {
      Serial.println("Activating garage door");
      ActivateDoor();
      redirect(client);
    }
    else
    {
      mainPage(client);
    }
  }
  else if(strstr(getLine, "GET /ajaxswitch") != NULL)
    {
      doorState(client);
   }
}
