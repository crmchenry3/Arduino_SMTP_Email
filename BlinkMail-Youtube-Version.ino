#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <SPI.h>

//
// Constants for this program
//
const int buttonPin = 2;    // the number of the pushbutton pin
const int ledPin =  8;      // the number of the LED pin
const int WAIT = 300;       // delay time

//
// Variables for this program.
//
// Variable for reading the digital status
int intButtonState = 0;
// ********************************  Arduino network information
// Your Ehternet Shield MAC address
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x23, 0x86 };
// Home IP address data (if unable to use DHCP)
byte ip[] = { 192, 168, 1, 108 };  
byte subnet[] = { 255, 255, 255, 0 };
byte gateway[] = { 192, 168, 1, 1 };
// Data for COMCAST SMTP mail server - NOTE *This changes*
byte smtpServerIP[] = { 76, 96, 62, 117 };
String smtpServerName = "smtp.comcast.net";
// Login data for COMCAST (already in MIME Base64 format)
String UserName64 = "dXNlcm5hbWU=";
String Password64 = "cGFzc3dvcmQ=";
// ********************************  Mail information
String Sender = "someone@comcast.net";
String Recipient = "others@theplace.com";
String Subject = "Arduino Button Click!";
String Body = "Button attached to digital input 2 pressed down.";
// ********************************  Arduino network connection
String ServerResponse="";
EthernetClient client;



//********************************************************
//** setup
//**   This is the first function to run and will initialize all
//**   components that are connected to the Arduino.
//********************************************************
void setup()
{
  Serial.begin(9600);  // for debug
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
   // get me some ip addressage!
  vodSetupCommunications();
}

//********************************************************
//** loop
//**   This is the main function to run and will act as
//**   the control loop for the Arduino.
//********************************************************
void loop()
{
  //
  // Read the state of the pushbutton value.
  //
  intButtonState = digitalRead(buttonPin);

  //
  // Check to see the pushbutton is pressed down.
  //   IF it is, the intButtonState is HIGH
  //      - turn on the LED for visual notification
  //      - send the vodSendEmail notification
  //
  if (intButtonState == HIGH)
  {     
    // Turn LED on.    
    digitalWrite(ledPin, HIGH);
    // Send the vodSendEmail notification.
    vodSendEmail();
    // Ensure that port is still open.
    vodTestConnection();
  } 
  else
  {
    // Turn LED off.
    digitalWrite(ledPin, LOW); 
  }
}


/***********************************************
        CALLED FUNCTIONS BELOW HERE
************************************************/


//********************************************************
//** vodSetupCommunications
//**   This function is called in order to initialize
//**   the communication with the ethernet port.  It
//**   gets the IP address using DHCP, but could use a
//**   specific IP address, subnet, and gateway.
//********************************************************
void vodSetupCommunications()
{

  // 
  // Variable for this function
  //
  byte byteIPByte;
  
  //
  // Attempt to establish connection to ethernet port.
  //
  Serial.println("Trying to connect to ethernet port ...");
//  if (!Ethernet.begin(mac))
//  //if (!Ethernet.begin(mac, ip, subnet, gateway))
//  {
//    Serial.println("Failed to establish DHCP connection.");
//    // No point in carrying on, so do nothing forevermore.
//    while(true);
//  }
  /*  Non-DHCP : while ( !Ethernet.begin(mac, ip, subnet, gateway) )  */
  while ( !Ethernet.begin(mac) )
  {
    Serial.println("Failed to establish DHCP connection.");
	Serial.println("retry ...");
  }
  // Pause to ensure successful connection.
  delay(WAIT);
  
  //
  // Display the established IP address for debugging
  //
  Serial.print("My IP address: ");
  for (byteIPByte = 0; byteIPByte < 4; byteIPByte++) {
    // Print the value of each byte of the IP address.
    Serial.print(Ethernet.localIP()[byteIPByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  Serial.println("***************************************");
  Serial.println();
}

//********************************************************
//** vodSendEmail
//**   This function will connect to the SMTP server and
//**   send the email message.
//********************************************************
void vodSendEmail()
{
    Serial.println("Connecting to the SMTP server ...");
	
	//
	//  If successful connecting to the SMTP server on
	//  port 587, then pass it the information needed
	//  to send the email message.  Otherwise, display
	//  a failure message for debugging.
	//
    if (client.connect(smtpServerIP, 587))
    {
      Serial.println("Connected to the SMTP server ...");
      
      vodEthernetOut("HELO " + smtpServerName); /* say hello*/
      vodEthernetOut("AUTH LOGIN ");
      vodEthernetOut(UserName64); /* Username*/
      vodEthernetOut(Password64); /* Password */
      vodEthernetOut("MAIL From:<" + Sender +">"); /* identify sender */
      vodEthernetOut("RCPT To:<" + Recipient + ">"); /* identify recipient */
      vodEthernetOut("DATA");
      vodEthernetOut("To: " + Recipient); /*  recipient in message header */
      vodEthernetOut("From: " + Sender); /* seder name in message header */
      vodEthernetOut("Subject: "+ Subject); /* insert subject */
      vodEthernetOut(""); /* empty line */
      vodEthernetOut(Body); /* insert body */
      vodEthernetOut(""); /* empty line */
      vodEthernetOut("."); /* end mail */
      vodEthernetOut("QUIT"); /* terminate connection */
      client.println();
    }
    else
    {
      Serial.println("The SMTP connection has failed!");
    }
}

//********************************************************
//** vodEthernetOut
//**   This function will write to the ethernet port
//**   and then display the data written as well as any
//**   response for debugging purposes.
//********************************************************
 void vodEthernetOut(String m) {
   // Write the string to the ethernet port.
   client.println(m);
   // Display the string that was written.
   Serial.println(">>>" + m);
   // Pause in order to wait for a response.
   delay(WAIT);
   // Read and display any response.
   vodGetResponse();
 }
 
//********************************************************
//** vodGetResponse
//**   This function look for and display any server
//**   response for debugging purposes.
//********************************************************
  void vodGetResponse() {
    if (client.available()) {
         char c = client.read();
         while (client.available()) { // Store command char by char.
           ServerResponse +=c;
           c = client.read();
         }
    Serial.println("<<<" + ServerResponse);
    ServerResponse="";
  }
 }
 
void vodTestConnection()
{
  if (!client.connected())
  {
    Serial.println();
    Serial.println("Port closed ... reconnecting ...");
    client.stop();
    delay(WAIT);
    vodSetupCommunications();
  }
}

