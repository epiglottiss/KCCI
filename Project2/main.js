"use strict";
const Mfrc522 = require("./rfid/mfrc522-rpi/index");
const SoftSPI = require("./rfid/mfrc522-rpi/node_modules/rpi-softspi");

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
const mfrc522 = new Mfrc522(softSPI1).setResetPin(22);
const mfrc522_2 = new Mfrc522(softSPI2).setResetPin(22);

var LCD = require('lcdi2c');
var lcd1 = new LCD( 1, 0x26, 16, 2 );
var lcd2 = new LCD( 1, 0x27, 16, 2 );

var express = require('express');
var app = express();
var bodyParser = require('body-parser');
var mysql = require('mysql');
var urlencodedParser = bodyParser.urlencoded({ extended: false });

var con = mysql.createConnection({
  host: "localhost",                //서버주소
  user: "testID",                   
  password: "1234",                 
  database: "Project2"        
});
con.connect(function(err) {
        if (err) throw err;
        console.log("DB Connected!");
});

let IncompleteFlag=0;
lcd1.clear();
lcd2.clear();
//DB에서 미완료건 있는지 검사
var orderTaker = [];
setInterval(function() {
    //미완료건이 생산중이면 검사 안함
    if(IncompleteFlag != 0)
        return;

    con.query("SELECT count(*) as CompleteCheck FROM 주문 where 완료여부=0", function (err, result,   fields) {
        if (err) throw err;
        //console.log("Checked InCompleted! Result is");
        //console.log(result);
        IncompleteFlag = result[0].CompleteCheck;
        if (IncompleteFlag != 0) {
            lcd1.clear();
            lcd1.setCursor(0, 0);
            lcd1.print('To Make:');
            lcd1.setCursor(8, 0);
            lcd1.print(IncompleteFlag.toString());
            lcd1.setCursor(0, 1);
            lcd1.print('Load An Item');
            console.log("lcd26 submitted");
            //조건에 맞춰서 생산할 주문한개 가져옴

            con.query("SELECT * FROM 주문 where 완료여부=0 order by 주문id limit 1;", function (err, result, fields) {
                if (err) throw err;
                for (var i = 0; i < result.length; i++) {
                    orderTaker.push(result[i]);
                }
            });
        }
    //orderTaker[0].열이름
    });
},1000);

setInterval(function() {
    //미완료건 없으면 rfid 인식안함
  if(IncompleteFlag == 0)
        return;

  mfrc522.reset();
    //console.log("Waiting for Manufacturing. Please load an item on conveyer!");
  //# Scan for cards
  let response = mfrc522.findCard();

    if (!response.status) {
        
        return;
    }

  console.log("Start Manufacturing!");

  //# Get the UID of the card
  response = mfrc522.getUid();

  if (!response.status) {
    console.log("UID Scan Error");
    return;
  }
  //# If we have the UID, continue
  const uid = response.data;
  //console.log(
  //  "Card read UID: %s %s %s %s",
  //  uid[0].toString(16),
  //  uid[1].toString(16),
  //  uid[2].toString(16),
  //  uid[3].toString(16)
  //);

  ////# Select the scanned card
  const memoryCapacity = mfrc522.selectCard(uid);

  //console.log("Card Memory Capacity: " + memoryCapacity);

  //# This is the default key for authentication
  const key = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff];

  //# Authenticate on Block 8 with key and uid
  if (!mfrc522.authenticate(4, key, uid)) {
    console.log("Authentication Error");
    return;
  }

    //생산 시작된 카드일 경우 다시 찍히는 상황 방지
    if (parseInt(mfrc522.getDataForBlock(4)[0]) != 0xff) {
        console.log("Not yet completed card!");
        return;
    }



  //# Variable for the data to write
  let data = [
    0xff,   //종류    0 
    0xff,   //수량    1
    0xff,   //수량2   2
    0xff,   //주문연도  3
    0xff,   //주문월   4
    0xff,   //주문일   5
    0xff,   //주문시간  6
    0xff,   //주문분  7
    0xff,   //주문id1  8
    0xff,   //주문id2  9
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff    
  ];
    console.log(orderTaker);
    if( orderTaker[0].종류 =="A")
        data[0] = 0x0a;
    else
        data[0] = 0x0b;
    var orderQuantity=parseInt(orderTaker[0].수량);
    data[1] = orderQuantity <<8;
    data[2] = orderQuantity % 0xff;
    data[3] = orderTaker[0].주문일시.getYear() - 100;

    console.log(data[3]);
    data[4] = orderTaker[0].주문일시.getMonth()+1;
    console.log(data[4]);
    data[5] = orderTaker[0].주문일시.getDate()-1;
    console.log(data[5]);

    data[6] = orderTaker[0].주문일시.getHours();
    console.log(data[6]);

    data[7] = orderTaker[0].주문일시.getMinutes();
    console.log(data[7]);

    data[8] = orderTaker[0].주문id << 8;
    data[9] = orderTaker[0].주문id % 0xff;

    mfrc522.writeDataToBlock(4, data);
  
  console.log("Block: 4 Data: " + mfrc522.getDataForBlock(4));

  //# Stop
  mfrc522.stopCrypto();
    IncompleteFlag=0;

}, 500);




setInterval(function () {

  //# reset card
  mfrc522_2.reset();

  //# Scan for cards
  let response = mfrc522_2.findCard();

  if (!response.status) {
    return;
  }
  console.log("Finishing RFID start!");

  //# Get the UID of the card
  response = mfrc522_2.getUid();
  if (!response.status) {
    console.log("UID Scan Error");
    return;
  }
  // If we have the UID, continue
  const uid = response.data;
  //console.log(
  //  "Card read UID: %s %s %s %s",
  //  uid[0].toString(16),
  //  uid[1].toString(16),
  //  uid[2].toString(16),
  //  uid[3].toString(16)
  //);

  //# Select the scanned card
  const memoryCapacity = mfrc522_2.selectCard(uid);

  //console.log("Card Memory Capacity: " + memoryCapacity);

  //# This is the default key for authentication
  const key = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff];

  //# Authenticate on Block 4 with key and uid
  if (!mfrc522_2.authenticate(4, key, uid)) {
    console.log("Authentication Error");
    return;
  }

    //생산 시작된 카드가 아니면 반환
    if (mfrc522_2.getDataForBlock(4)[0] == 0xff) {
        console.log("It's raw card!");
        return;
    }

    var orderId = ((mfrc522_2.getDataForBlock(4)[8] << 8) + mfrc522_2.getDataForBlock(4)[9]).toString(10);
    
    var sqlForUpdate = "UPDATE 주문 set 완료여부=1 WHERE 주문id =" + orderId;
    console.log("sql for update is");
    console.log(sqlForUpdate);
    con.query(sqlForUpdate, function (err, result, fields) {
        if (err) throw err;
        console.log("Updated Completed!");
    });

    //# Variable for the data to write
  let data = [
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff
  ];
    
  
  mfrc522_2.writeDataToBlock(4, data);
    
  //# Dump Block 4
    console.log("Block: 4 Data: " + mfrc522_2.getDataForBlock(4));

    lcd2.clear();
    lcd2.setCursor(0, 0);
    lcd2.print("order Id is");
    lcd2.setCursor(0, 1);
    lcd2.print(orderId);

  //# Stop
  mfrc522_2.stopCrypto();
}, 500);


//이미지경로 설정
app.use(express.static(__dirname + "/sub/img" ));

//페이지 이동
app.get('/',function (req, res) {
   res.sendFile( __dirname + "/" + "main.html" );
})
app.get('/main.html', function (req, res) {
   res.sendFile( __dirname + "/" + "main.html" );
})
app.get('/order.html', function (req, res) {
   res.sendFile( __dirname + "/" + "order.html" );
})
app.get('/info.html', function (req, res) {
   res.sendFile( __dirname + "/" + "info.html" );
})
app.get('/menu.html', function (req, res) {
   res.sendFile( __dirname + "/" + "menu.html" );
})
app.get('/orderCheck.html', function (req, res) {
   res.sendFile( __dirname + "/" + "orderCheck.html" );
})


//post방식으로 order에서 받아오고, orderCompleted.ejs로 브라우저에 보여줌
app.post('/orderCompleted.ejs', urlencodedParser, function (req, res) {

    var name=req.body.name;
    var phone=req.body.phone;
    var address=req.body.address;
    var stuff=req.body.stuff;
    var quantity=req.body.quantity;
    let today = new Date();
    
    console.log("Parsed!");
    res.render(__dirname + "/orderCompleted.ejs", {name:name, phone:phone, address:address,stuff:stuff,quantity:quantity, orderDate:today});
    console.log("Rendered!");
    /*console.log(response);*/
    res.end();
    console.log("ended!");
  
    
    //db연결
    var sql = "INSERT INTO 주문 (주문자,전화번호,주소,종류,수량,주문일시) VALUES (?);";
    var values = [name,phone,address,stuff,quantity,today];
    con.query(sql, [values], function (err, result) {
       console.log("query started");
       if (err) throw err;
       console.log("Number of records inserted: " + result.affectedRows);   
   
    });
})


//주문내역 검색하면 DB에서 가져와서 보여줌
app.post('/showOrderCheck.ejs', urlencodedParser, function (req, res) {

    var name=req.body.name;
    var phone=req.body.phone;
    
    var sql = "SELECT * FROM 주문;";
    var resultTaker=[];
     
    con.query(sql, function (err, result, fields) {
       console.log("query started");
        if (err) throw err;
        for (var i=0; i<result.length; i++) {
            resultTaker.push(result[i]);
        }
        //console.log(resultTaker);
        //console.log(result);
        //var resultTaker = JSON.stringify(result);
        //resultTaker = copyObject(result);
        //resultTaker = Object.assign({},result);
        //resultTaker = new Object();
        //resultTaker = result;
        //resultTaker = JSON.stringify(result);
        //resultTaker = result;
        //resultTaker = JSON.parse(JSON.stringify(result));
    });

    //node.JS의 비동기성때문에 쿼리실행함수와 시간을 맞춰야함. setTimeout은 원하는 시간만큼 나중에 실행됨
    setTimeout(function() { 
        res.render(__dirname + "/showOrderCheck.ejs", {result:resultTaker});
    console.log("rendered!");
    res.end();    
    console.log("ended!");
    console.log(resultTaker);}, 10); 
});

var server = app.listen(8081, function () {
   var host = server.address().address
   var port = server.address().port
   
   console.log("Example app listening at http://%s:%s", host, port)
})