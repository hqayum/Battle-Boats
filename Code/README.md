All the files for this project.

**Files I implemented:**
* Protocol.c - which is the backend work, it allows communication via ASCII messages. We can encode protocol messages in this file, and
  then decode them using a Finite State Machine when decoding is needed. The ASCII messages come in the form of
  '$MESSAGE_ID,DATA1,DATA2,DATA3,\*XX\n' (DATA3 is only needed for hit messages). '$' indicates the start of the ASCII message, the
  'MESSAGE_ID' is a three character string encoding the type of message, 'DATA' is the data encoded as ASCII characters, '\*XX' which the
  \* indicates as the start of the two seperate ASCII hexadecimal characters which we get from XORing all bytes between '$' and '\*', and
  '\n' indicates the new line character meaning the end of of our ASCII message.
* Tester.c - test file for Protocol.c

**Files partner implemented:**
* Field.c - frontend work, what you see on the OLED like the boats and if a guess hits a boat and the entire field on the OLED.
* Leds.h - allows control for Leds on the UNO32.

**Files we both implemented together:**
* ArtificialAgent.c - artificial intelligence agent, the main Finite State Machine was implemented in this file.
