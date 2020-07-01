"use strict";
const Mfrc522 = require("./../index");
const SoftSPI = require("rpi-softspi");

console.log("scanning...");

const softSPI1 = new SoftSPI({
  clock: 23, // pin number of SCLK
  mosi: 19, // pin number of MOSI
  miso: 21, // pin number of MISO
  client: 24 // pin number of CS
});
const softSPI2 = new SoftSPI({
  clock: 23, // pin number of SCLK
  mosi: 19, // pin number of MOSI
  miso: 21, // pin number of MISO
  client: 26 // pin number of CS
});

// GPIO 24 can be used for buzzer bin (PIN 18), Reset pin is (PIN 22).
// I believe that channing pattern is better for configuring pins which are optional methods to use.
const mfrc522 = new Mfrc522(softSPI1).setResetPin(22);
const mfrc522_2 = new Mfrc522(softSPI2).setResetPin(22);

setInterval(function() {
  //# reset card
  mfrc522.reset();

  //# Scan for cards
  let response = mfrc522.findCard();

  if (!response.status) {
    console.log("No Card 1");
    return;
  }
  console.log("Card detected, CardType: " + response.bitSize);

  //# Get the UID of the card
  response = mfrc522.getUid();

  if (!response.status) {
    console.log("UID Scan Error");
    return;
  }
  //# If we have the UID, continue
  const uid = response.data;
  console.log(
    "Card read UID: %s %s %s %s",
    uid[0].toString(16),
    uid[1].toString(16),
    uid[2].toString(16),
    uid[3].toString(16)
  );

  //# Select the scanned card
  const memoryCapacity = mfrc522.selectCard(uid);

  console.log("Card Memory Capacity: " + memoryCapacity);

  //# This is the default key for authentication
  const key = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff];

  //# Authenticate on Block 8 with key and uid
  if (!mfrc522.authenticate(8, key, uid)) {
    console.log("Authentication Error");
    return;
  }

  //# Dump Block 8
  console.log("Block: 8 Data: " + mfrc522.getDataForBlock(8));

  //# Stop
  mfrc522.stopCrypto();
}, 500);




setInterval(function() {
  //# reset card
  mfrc522_2.reset();

  //# Scan for cards
  let response = mfrc522_2.findCard();

  if (!response.status) {
    console.log("No Card 2");
    return;
  }
  console.log("Card detected, CardType: " + response.bitSize);

  //# Get the UID of the card
  response = mfrc522_2.getUid();

  if (!response.status) {
    console.log("UID Scan Error");
    return;
  }
  //# If we have the UID, continue
  const uid = response.data;
  console.log(
    "Card read UID: %s %s %s %s",
    uid[0].toString(16),
    uid[1].toString(16),
    uid[2].toString(16),
    uid[3].toString(16)
  );

  //# Select the scanned card
  const memoryCapacity = mfrc522_2.selectCard(uid);

  console.log("Card Memory Capacity: " + memoryCapacity);

  //# This is the default key for authentication
  const key = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff];

  //# Authenticate on Block 8 with key and uid
  if (!mfrc522_2.authenticate(8, key, uid)) {
    console.log("Authentication Error");
    return;
  }

  //# Dump Block 8
  console.log("Block: 8 Data: " + mfrc522_2.getDataForBlock(8));

  //# Stop
  mfrc522_2.stopCrypto();
}, 500);






