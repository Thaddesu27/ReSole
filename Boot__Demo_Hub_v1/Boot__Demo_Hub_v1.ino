#include "BluetoothSerial.h"
int buttonPins [10] = {32, 5, 33, 18, 26, 21, 25, 22, 27, 19};
char *cmdNames[5] = {"Reset", "Sync Times", "Start Running", "Stop Running", "Transfer Data"};
#include "heltec.h"
#include "images.h"
BluetoothSerial SerialBT;
//BluetoothSerial SerialBT;
char serNum[19]; // Holds Device Serial Number - 18 chars + null dor string
int snI = 0;
// another comment another
// Added comment for GitHub check
struct rppDelayStruct
{
  // my mnew data structure
  char sn[20];
  long ms;
  bool leftLeg;
  bool rppConnected;
} rppDelay[10];

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Client Connected");
  }
  if (event == ESP_SPP_CLOSE_EVT ) {
    Serial.println("Client disconnected");
  }
}
SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, RST_OLED);
void setup()
{

  //strcpy(rppDelay[0].sn, "RPP140721001216044");
  //strcpy(rppDelay[1].sn, "RPP140721001217044");
  Heltec.begin(true, false, true);
  Wire.begin(SDA_OLED, SCL_OLED); //Scan OLED's I2C address via I2C0
  display.init();
  display.clear();
  display.display();
  //display.flipScreenVertically();
  display.screenRotate(ANGLE_0_DEGREE);
  display.display();
  logo();
  display.display();
  delay(3000);
  display.clear();
  strcpy(rppDelay[0].sn, "RPP110521001085066");
  strcpy(rppDelay[1].sn, "RPP110521001086066");
  
  display.screenRotate(ANGLE_0_DEGREE);
  display.display();
  // put your setup code here, to run once:
  oled(1, "RE:BOOT");
  oled(2, "Initialising");
  
  Serial.begin(115200);
  delay(1000);
  Serial.println("Ready");

  SerialBT.register_callback(callback);
  oled(0, "");
  oled(2, "Looking for Boots");
  oled(3, "Unplug from Charger");
  waitForTwoRPPs();
  oled(0, "");
  oled(2, "Found Boots");
  delay(1000);
  oled(2, "Found Boots - Sync'd");
}

void logo() {
  display.clear();
  display.drawXbm(0, 5, logo_width, logo_height, (const unsigned char *)logo_bits);
  display.display();
}
int rppIndex = 0;
void loop() {
  // put your main code here, to run repeatedly:
  char x;

  switch (button())
  {
    case -1:
      break;
    case 1:
      Serial.println("1 pressed - Reset ESP");
      oled(0, "");
      oled(3, "Rebooting Hub");
      delay(1000);
      rebootToConnect();
    case 2:
      oled(0, "");
      oled(2, "Syncing");
      oled(3, "Stay Close to Hub");
      oled(5, "(LED White when Sync'd)");
      for (int i = 0; i < 2; i++)
      {
        connectTo(i);
        Serial.println("2 pressed - Sync Times");
        SerialBT.print("2");
        Serial.print("Sending <");
        Serial.print(millis());
        Serial.println(">");
        SerialBT.println(millis());
        disconnect();
      }

      setLights(LOW, LOW, HIGH, LOW, LOW);
      break;
    case 3:
      oled(0, "");
      oled(2, "Ready to start Running");
      oled(3, "Stay Close to Hub");
      oled(5, "(Flash Green/Then Off)");
      for (int i = 0; i < 2; i++)
      {
        connectTo(i);
        SerialBT.print("3");
        Serial.println("3 pressed - Start");
        disconnect();

        setLights(LOW, LOW, LOW, HIGH, LOW);
      }
      break;
    case 4:
      oled(0, "");
      oled(2, "Run Stopping");
      oled(3, "Stay Close to Hub");
      oled(5, "(LED Purple when stopped)");
      for (int i = 0; i < 2; i++)
      {
        connectTo(i);
        SerialBT.print("4");
        Serial.println("4 pressed - Stop");
        disconnect();
      }
      setLights(LOW, LOW, LOW, LOW, HIGH);
      break;
    case 5:
      oled(0, "");
      oled(2, "Transfering Data");
      oled(3, "Stay Close to Hub");
      for (int i = 0; i < 2; i++)
      {
        connectTo(i);
        SerialBT.print("5");
        Serial.println("5 pressed - Transfer");
        //justListen();
        receiveData();
        oled(5, "DONE");
        Serial.println("\nTransfer from RPP Complete");
        disconnect();
      }
      setLights(HIGH, HIGH, HIGH, HIGH, HIGH);
      break;
    default:
      Serial.println("Unknown Button");
      break;
  }

  while (SerialBT.available())
  {
    x = SerialBT.read();
    if (snI < 18)
      serNum[snI++] = x;
    else
      serNum[18] = '\0';
    if (x < 48)
    {
      if (x < 16) Serial.print("0");
      Serial.print(x, HEX);
    }
    else
    {
      Serial.print(x);
    }
  }
}

int button()
{
  bool aPress = false;
  int cmd = -1;
  /*
    Serial.print(digitalRead(buttonPins[0 * 2]));
    Serial.print(digitalRead(buttonPins[1 * 2]));
    Serial.print(digitalRead(buttonPins[2 * 2]));
    Serial.print(digitalRead(buttonPins[3 * 2]));
    Serial.print(digitalRead(buttonPins[4 * 2]));
    Serial.println(cmd);
  */
  for (int i = 0; i < 5; i++)
  {
    if (digitalRead(buttonPins[i * 2]) == HIGH)
    {
      //digitalWrite(buttonPins[i * 2 + 1], HIGH);
    }
    else
    {
      delay(100);
      if (digitalRead(buttonPins[i * 2]) == HIGH)
      {
        Serial.print("\nSending Command ");
        Serial.println(cmdNames[i]);
        aPress = true;
        cmd = i + 1;
      }
    }
  }
  return cmd;
}


void syncTime()
{
  delay(100);
  if (digitalRead(26) == HIGH)
  {
    Serial.println("Sending Sync Command");
    SerialBT.print("2");
    Serial.print("Sending <");
    Serial.print(millis());
    Serial.println(">");
    SerialBT.println(millis());
  }
}

void receiveData()
{
  char x;
  int lines = 0;
  //Serial.println("Permanently Listening"); justListen();
  Serial.println("Waiting for Data");
  while (!SerialBT.available());
  delay(100);
  Serial.println("Receiving Data");
  Serial.print("TX_START\nExpecting ");
  bool quit = false;
  while (1)
  {
    while (SerialBT.available())
    {
      x = SerialBT.read();
      if (x == 0xFA)
      {
        quit = true;
        break;
      }
      if (x == 10) continue;
      if (x == 13)
      {
        if (lines == 0)
          Serial.print(" records. \nAppling delay ");
        if (lines == 1)
          Serial.print(" ms.");
        if (lines == 0)
        {
          lines++;
          continue;
        }
        Serial.print("\nRecord ");
        Serial.print((lines));
        Serial.print(": ");
        lines++;

        continue;
      }
      if (x < 32)
      {
        if (x < 16) Serial.print("0");
        Serial.print(x, HEX);
      }
      else
      {
        Serial.print(x);
      }
    }
    if (quit)
    {
      Serial.println("TX_END");
      break;
    }
  }
}

void justListen()
{
  char x;
  while (1)
  {
    while (SerialBT.available())
    {
      {
        x = SerialBT.read();
        if (x == 10) continue;
        if (x == 13) Serial.println();
        if (x < 32)
        {
          if (x < 16) Serial.print("0");
          Serial.print(x, HEX);
        }
        else
        {
          Serial.print(x);
        }
      }
    }
  }
}

void rebootToConnect()
{
  // ToDo Clean up and set LEDs before going to sleep
  esp_sleep_enable_timer_wakeup(1);
  esp_deep_sleep_start(); // Send In:Sight to sleep
}

void lightTest()
{
  for (int j = 0; j < 5; j++)
  {
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(buttonPins[i * 2 + 1], HIGH);
      delay(250);
      digitalWrite(buttonPins[i * 2 + 1], LOW);
    }
  }
}

void setLights(int a, int b, int c, int d, int e)
{
  digitalWrite(buttonPins[0 * 2 + 1], a);
  digitalWrite(buttonPins[1 * 2 + 1], b);
  digitalWrite(buttonPins[2 * 2 + 1], c);
  digitalWrite(buttonPins[3 * 2 + 1], d);
  digitalWrite(buttonPins[4 * 2 + 1], e);
}

void waitForTwoRPPs()
{
  bool found[2] = {false, false};
  int looping = 0;
  while ((!found[0]) && (!found[1]))
  {
    Serial.print(looping++);
    Serial.print(":\t");
    for (int i = 0; i < 2; i++)
    {
      if (!found[i])
      {
        Serial.print("Looking for "); Serial.println(rppDelay[i].sn);
        if (!SerialBT.begin("PhysiGo_Hub", true))
          Serial.println("Failed to connect to BT"); //Bluetooth device name of Secondary
        found[i] = SerialBT.connect(rppDelay[i].sn);
        if (found[i])
        {
          Serial.print("Connected succesfully to ");
          Serial.println(rppDelay[i].sn);
          SerialBT.print("K");
          delay(1000);
          SerialBT.flush();
          SerialBT.disconnect();
        }
        else
        {
          Serial.print("STILL looking for "); Serial.println(rppDelay[i].sn);
        }
      }
    }
    delay(1000);
  }
  Serial.println("Found both RPPs");
}

void connectTo(int rpp)
{
  rppDelay[rpp].rppConnected = false;

  while (!rppDelay[rpp].rppConnected)
  {
    Serial.print("Connecting to "); Serial.println(rppDelay[rpp].sn);
    /*
      if (!SerialBT.begin("PhysiGo_Hub", true))
      Serial.println("Failed to connect to BT"); //Bluetooth device name of Secondary
    */
    rppDelay[rpp].rppConnected = SerialBT.connect(rppDelay[rpp].sn);
    if (rppDelay[rpp].rppConnected)
    {
      Serial.print("Reconnected succesfully to ");
      Serial.println(rppDelay[rpp].sn);

      SerialBT.println("A");
      SerialBT.println("B");
      SerialBT.println("C");
      SerialBT.println("D");
      SerialBT.println("E");
      SerialBT.println("F");

      break;
    }
    else
    {
      Serial.print("STILL looking for "); Serial.println(rppDelay[rpp].sn);
    }
    //delay(1000);
  }
}

void disconnect()
{
  SerialBT.flush();
  SerialBT.disconnect();
}

void oled(int line, char * msg)
{
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setColor(BLACK); // alternate colors
  display.fillRect(0, 40, 128, 10);
  display.setColor(WHITE); // alternate colors
  display.drawString(60, 0, "RE:RUN");
  if (line == 0) display.clear();
  if (line == 1) display.drawString(60, 0, msg);
  if (line == 2) display.drawString(60, 10, msg);
  if (line == 3) display.drawString(60, 20, msg);
  if (line == 4) display.drawString(60, 30, msg);
  if (line == 5) display.drawString(60, 40, msg);
  display.drawString(60, 0, "RE:RUN");
  display.display();
}
