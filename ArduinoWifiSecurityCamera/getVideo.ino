/*************************************************** 
  This is a code for the TTL JPEG Camera (VC0706 chipset)
  Camera Functions are adopted from VC0706 library
  Written by Limor Fried/Ladyada for Adafruit Industries. 
  -----> https://github.com/adafruit/Adafruit-VC0706-Serial-Camera-Library

  For display messages Spark Core USB serial port is used to communicate.

  Yazdan Shirvany
  y_shirvany@yahoo.com  
****************************************************/

#define VC0706_RESET  0x26
#define VC0706_GEN_VERSION 0x11
#define VC0706_SET_PORT 0x24
#define VC0706_READ_FBUF 0x32
#define VC0706_GET_FBUF_LEN 0x34
#define VC0706_FBUF_CTRL 0x36
#define VC0706_DOWNSIZE_CTRL 0x54
#define VC0706_DOWNSIZE_STATUS 0x55
#define VC0706_READ_DATA 0x30
#define VC0706_WRITE_DATA 0x31
#define VC0706_COMM_MOTION_CTRL 0x37
#define VC0706_COMM_MOTION_STATUS 0x38
#define VC0706_COMM_MOTION_DETECTED 0x39
#define VC0706_MOTION_CTRL 0x42
#define VC0706_MOTION_STATUS 0x43
#define VC0706_TVOUT_CTRL 0x44
#define VC0706_OSD_ADD_CHAR 0x45

#define VC0706_STOPCURRENTFRAME 0x0
#define VC0706_STOPNEXTFRAME 0x1
#define VC0706_RESUMEFRAME 0x3
#define VC0706_STEPFRAME 0x2

#define VC0706_640x480 0x00
#define VC0706_320x240 0x11
#define VC0706_160x120 0x22

#define VC0706_MOTIONCONTROL 0x0
#define VC0706_UARTMOTION 0x01
#define VC0706_ACTIVATEMOTION 0x01

#define VC0706_SET_ZOOM 0x52
#define VC0706_GET_ZOOM 0x53

#define CAMERABUFFSIZ 100
#define CAMERADELAY 10
// Camera Setting
uint8_t  serialNum;
uint8_t  camerabuff[CAMERABUFFSIZ+1];
uint8_t  bufferLen;
uint16_t frameptr;

// TCP Setting
TCPClient client;
byte server[] = {95, XX, XX, XX };  // Add your Server IP
byte c;
int port = 80;
int led  = D7;
int sparkTempF = 0;

//---------------------- Low Level Commands
void common_init(void) {
  frameptr  = 0;
  bufferLen = 0;
  serialNum = 0;
}

void sendCommand(uint8_t cmd, uint8_t args[] = 0, uint8_t argn = 0) {
    Serial1.write((byte)0x56);
    Serial1.write((byte)serialNum);
    Serial1.write((byte)cmd);

    for (uint8_t i=0; i<argn; i++) {
      Serial1.write((byte)args[i]);
      //Serial.print(" 0x");
      //Serial.print(args[i], HEX);
    }

}
uint8_t readResponse(uint8_t numbytes, uint8_t timeout) {
  uint8_t counter = 0;
  bufferLen = 0;
  int avail;

  while ((timeout != counter) && (bufferLen != numbytes)){
    avail = Serial1.available();
    if (avail <= 0) {
      delay(1);
      counter++;
      continue;
    }
    counter = 0;
    // there's a byte!
    camerabuff[bufferLen++] = Serial1.read();
  }
  //printBuff();
//camerabuff[bufferLen] = 0;
//Serial.println((char*)camerabuff);
  return bufferLen;
}

boolean verifyResponse(uint8_t command) {
  if ((camerabuff[0] != 0x76) ||
      (camerabuff[1] != serialNum) ||
      (camerabuff[2] != command) ||
      (camerabuff[3] != 0x0))
      return false;
  return true;

}

boolean runCommand(uint8_t cmd, uint8_t *args, uint8_t argn,
			   uint8_t resplen, boolean flushflag) {
  // flush out anything in the buffer?
  if (flushflag) {
    readResponse(100, 10);
  }

  sendCommand(cmd, args, argn);
  if (readResponse(resplen, 200) != resplen)
    return false;
  if (! verifyResponse(cmd))
    return false;
  return true;
}


//---------- Camera Functions

boolean camReset() {
  uint8_t args[] = {0x0};

  return runCommand(VC0706_RESET, args, 1, 5, true);
}


boolean camBegin(uint16_t baud) {
  Serial1.begin(baud);
  return camReset();
}
uint8_t camGetImageSize() {
  uint8_t args[] = {0x4, 0x4, 0x1, 0x00, 0x19};
  if (! runCommand(VC0706_READ_DATA, args, sizeof(args), 6, true))
    return -1;

  return camerabuff[5];
}

boolean camSetImageSize(uint8_t x) {
  uint8_t args[] = {0x05, 0x04, 0x01, 0x00, 0x19, x};

  return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5, true);
}

boolean cameraFrameBuffCtrl(uint8_t command) {
  uint8_t args[] = {0x1, command};
  return runCommand(VC0706_FBUF_CTRL, args, sizeof(args), 5, false);
}

boolean camTakePicture() {
  frameptr = 0;
  return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME);
}

boolean camRsumeVideo() {
  return cameraFrameBuffCtrl(VC0706_RESUMEFRAME);
}

uint32_t camFrameLength(void) {
  uint8_t args[] = {0x01, 0x00};
  if (!runCommand(VC0706_GET_FBUF_LEN, args, sizeof(args), 9, true))
    return 0;

  uint32_t len;
  len = camerabuff[5];
  len <<= 8;
  len |= camerabuff[6];
  len <<= 8;
  len |= camerabuff[7];
  len <<= 8;
  len |= camerabuff[8];

  return len;
}

uint8_t camAvailable(void) {
  return bufferLen;
}


uint8_t * camReadPicture(uint8_t n) {
  uint8_t args[] = {0x0C, 0x0, 0x0A,
                    0, 0, frameptr >> 8, frameptr & 0xFF,
                    0, 0, 0, n,
                    CAMERADELAY >> 8, CAMERADELAY & 0xFF};

  if (! runCommand(VC0706_READ_FBUF, args, sizeof(args), 5, false))
    return 0;


  // read into the buffer PACKETLEN!
  if (readResponse(n+5, CAMERADELAY) == 0)
      return 0;


  frameptr += n;

  return camerabuff;
}


//----------- Setup the MicroController
void setup() {

  Spark.function("camera", cameraFunc);
//  Spark.function("blink", blinkFunc);
  Spark.variable("temperature", &sparkTempF, INT);
  Serial.begin(9600);
  Serial.println("VC0706 Camera snapshot test");
    // Begin Camera
      common_init();
    // Try to locate the camera
    if (camBegin(38400)) {
      Serial.println("Camera Found:");
    } else {
      Serial.println("No camera found?");
      return;
    }


}

// call the below function when the POST request matches it
int cameraFunc(String command) {
  Serial.println("Got the request...");
  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120
  // Remember that bigger pictures take longer to transmit!
  //  camSetImageSize(VC0706_640x480);        // biggest
      camSetImageSize(VC0706_320x240);        // medium
  //  camSetImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = camGetImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

  if (! camTakePicture())
    Serial.println("Failed to snap!");
  else
    Serial.println("Picture taken!");

  // Get the size of the image (frame) taken
  uint16_t jpglen = camFrameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  //--- TCP

// Prepare request
  String start_request = "";
  String end_request = "";
  start_request = start_request + "\n" + "--AaB03x" + "\n"
                                + "Content-Disposition: form-data; name=\"picture\"; filename=\"CAM.JPG\"" + "\n"
                                + "Content-Type: image/jpeg" + "\n" + "Content-Transfer-Encoding: binary" + "\n"
                                + "\n";
  end_request = end_request + "\n" + "--AaB03x--" + "\n";

  uint16_t extra_length;
  extra_length = start_request.length() + end_request.length();
  Serial.println("Extra length:");
  Serial.println(extra_length);

  uint16_t len = jpglen + extra_length;

   Serial.println("Full request:");
   Serial.println(F("POST /cameras HTTP/1.1"));
   Serial.println(F("Host: XX.XX.XX.XX:80"));   // Your Server IP
   Serial.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
   Serial.print(F("Content-Length: "));
   Serial.println(len);
   Serial.print(start_request);
   Serial.print("binary data");
   Serial.print(end_request);

   Serial.println("Starting connection to server...");

  if (command == "takePics") {
    Serial.println("Take Picture...");

  if (client.connect(server, port))
    {
        Serial.println(F("Connected !"));
        client.println(F("POST /cameras/img HTTP/1.1"));
        client.println(F("Host: XX.XX.XX.XX:80"));   // Your Server IP
        client.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
        client.print(F("Content-Length: "));
        client.println(len);
        client.println();

    int32_t time = millis();
    // Read all the data up to # bytes!
    byte wCount = 0; // For counting # of writes
    while (jpglen > 0) {
      // read 32 bytes at a time;
      uint8_t *buffer;
      uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
      buffer = camReadPicture(bytesToRead);
      Serial.println("Buffer data");
      client.write(buffer, bytesToRead);
      jpglen -= bytesToRead;
    }

    client.print(end_request);
    client.println();

    Serial.println("Transmission over");
    time = millis() - time;
    Serial.println("done!");
    Serial.print(time); Serial.println(" ms elapsed");

    client.stop();
    camRsumeVideo();
    return 0;  // Picture success
  }else {
    return -1; // Failed
  }

  }else if (command == "takeVideo"){
    Serial.println("Take Video...");

    while(1){
      Serial.println("In the Loop...");
      jpglen = camFrameLength();
  if (client.connect(server, port))
    {

        Serial.println(F("Connected !"));
        client.println(F("POST /cameras/video HTTP/1.1"));
        client.println(F("Host: XX.XX.XX.XX:80"));   // Your Server IP
        client.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
        client.print(F("Content-Length: "));
        client.println(len);
        client.println();

    int32_t time = millis();
    // Read all the data up to # bytes!
    byte wCount = 0; // For counting # of writes
    while (jpglen > 0) {
      // read 32 bytes at a time;
      uint8_t *buffer;
      uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
      buffer = camReadPicture(bytesToRead);
      Serial.println("Buffer data");
      client.write(buffer, bytesToRead);
      if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
        Serial.print('.');
        wCount = 0;
      }
      //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
      jpglen -= bytesToRead;

    }

    client.print(end_request);
    client.println();

    Serial.println("Transmission over");


    time = millis() - time;
    Serial.println("done!");
    Serial.print(time); Serial.println(" ms elapsed");
    //client.flush();
    client.stop();
    camRsumeVideo();
    return 1; // Video success
}

}

  }if (command == "stopVideo") {
    return 2; // Video Stop
  }else if (command == "reset"){
    Serial.println("Reset Camera...");
    camRsumeVideo();
    return 3;  // Reset
  }


}

void loop() {

}
