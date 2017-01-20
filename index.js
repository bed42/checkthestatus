var express = require('express');
var app = express();
var storage = require('node-persist');
var request = require('request');

app.get('/setstatus', function (req, res) {
    result = setStatus(req,res);
    res.send(result);
});

function setStatus(req, res){
  //expected params, deviceid (string) and status (0 = shut, 1 = open)
  console.log('deviceid: ' + req.query.deviceid);
  console.log('status: ' + req.query.status);

  if (req.query.deviceid && req.query.status) {
    storage.setItemSync(req.query.deviceid,req.query.status);
    result = ("OK - device " + req.query.deviceid + " set to " + req.query.status);
  } else {
    result = ("Missing Params deviceid and status: " + req.originalUrl);
  }
  return result;
}

app.get('/setstatuses', function (req, res) {
    var result = "";
    if(req.query.temperature && req.query.temperature.length > 0) {
      req.query.deviceid = 'Temperature';
      req.query.status = req.query.temperature;
      result += setStatus(req,res)+"\n";
    }
    if(req.query.humidity && req.query.humidity.length > 0) {
      req.query.deviceid = 'Humidity';
      req.query.status = req.query.humidity;
      result += setStatus(req,res)+"\n";
    }
    if(req.query.smoke && req.query.smoke.length > 0) {
      req.query.deviceid = 'Smoke';
      req.query.status = req.query.smoke;
      var existing_status = storage.getItemSync(req.query.deviceid);
      if(existing_status != req.query.smoke){
        pushSmokeUpdate(req.query.smoke, "@bed");
        pushSmokeUpdate(req.query.smoke, "@neetika");
      }

      result += setStatus(req,res)+"\n";
    }
    if(req.query.panic && req.query.panic.length > 0) {
      req.query.deviceid = 'Panic';
      req.query.status = req.query.panic;
      var existing_status = storage.getItemSync(req.query.deviceid);
      if((existing_status != req.query.panic) && (req.query.panic == '1')){
        pushPanic("@bed");
        pushPanic("@neetika");
      }

      result += setStatus(req,res)+"\n";
    }
    if(req.query.door && req.query.door.length > 0) {
      req.query.deviceid = 'Door';
      req.query.status = req.query.door;
      result += setStatus(req,res)+"\n";
    }
    if(req.query.light && req.query.light.length > 0) {
      req.query.deviceid = 'Light';
      req.query.status = req.query.light;
      result += setStatus(req,res)+"\n";
    }
    if(req.query.fan && req.query.fan.length > 0) {
      req.query.deviceid = 'Fan';
      req.query.status = req.query.fan;
      result += setStatus(req,res)+"\n";
    }
    res.send(result);
});

function pushSmokeUpdate(onfire, target){
  var json = onfire == '1' ? '"text": "OMG Smoke has been detected!.\nIs your house burning down?"' : '"text": "The smoke has cleared! Either the fire is out, or your house has completely burnt down."';
  if ( target ) {
    json += ', "channel": "'+ target +'"';
  }
  //json += ', "icon_url": "https://www.ncptt.nps.gov/wp-content/uploads/fire-vector.png"'
  json = "{" + json + "}";
  request({url:'https://hooks.slack.com/services/KEYGOESHERE', method:"POST", body: json}, function (error, response, body) {
    console.log("Smoke Update: " + body)
  })
}

function pushPanic(target){
  var json = '"text": "PANIC STATIONS!"';
  if ( target ) {
    json += ', "channel": "'+ target +'"';
  }
  //json += ', "icon_url": "http://i.imgur.com/wbMOuPC.png"'
  json = "{" + json + "}";
  console.log("Payload: " + json)
  request({url:'https://hooks.slack.com/services/KEYGOESHERE', method:"POST", body: json}, function (error, response, body) {
    console.log("Panic: " + body)
  })
}

//app.get('/setrequestedstatus', function (req, res) {
function setrequest(req,res){
  //expected params, deviceid (string) and status (0 = shut, 1 = open)
  console.log('deviceid: ' + req.query.deviceid);
  console.log('status: ' + req.query.status);

  if (req.query.deviceid && req.query.status) {
    if ((req.query.deviceid == 'light') || (req.query.deviceid == 'fan')){
      if ((req.query.status == '1') || (req.query.status == '0')){
        storage.setItemSync(req.query.deviceid+'-requested',req.query.status);
        res.send("OK -  " + req.query.deviceid + " will be requested to change to " + (req.query.status == '0' ? "off" : "on"));
      } else {
        res.send("Error: can only set to 1 or 0");
      }
    } else {
      res.send("Error: only light or fan can be requested");
    }
  } else {
    res.send("Missing Params deviceid and status: " + req.originalUrl);
  }
}
//});

app.get('/home', function (req, res) {
  //the main handler to the slack command
  if(req.query.text == 'check') {
      check(req,res);
  } else if(req.query.text == 'light on') {
      req.query.deviceid = 'light';
      req.query.status = '1';
      setrequest(req,res);
  } else if(req.query.text == 'light off') {
        req.query.deviceid = 'light';
        req.query.status = '0';
        setrequest(req,res);
      } else if(req.query.text == 'fan on') {
          req.query.deviceid = 'fan';
          req.query.status = '1';
          setrequest(req,res);
      } else if(req.query.text == 'fan off') {
            req.query.deviceid = 'fan';
            req.query.status = '0';
            setrequest(req,res);
  } else {
      //res.send(req.originalUrl)
      res.send("Home Check Usage:\n /home check - check the current status at home\n /home light on/off - the light on or off\n /home fan on/off - turn the fan on or off");
  }
});

function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1).toLowerCase();
}
//app.get('/checkthestatus', function (req, res) {
function check(req,res){
  //the handler to the slack command
  var keys = storage.keys();
  var result = "Here is the current status:\n";
  console.log('checkthestatus command received. Available Keys: ' + keys);
  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if((key.indexOf("-requested") == -1) && (key != "Panic")){
      key = capitalizeFirstLetter(key);
      var status = storage.getItemSync(keys[i]);
      if(key.toLowerCase() == 'door'){
         result += " • " + key + " is " + (status == 0 ? "closed" : "open") + "\n";
      } else if(key.toLowerCase() == 'smoke'){
         result += " • " + key + " is " + (status == 0 ? "not detected" : "DETECTED!") + "\n";
       } else if(key.toLowerCase() == 'light' || key.toLowerCase() == 'fan'){
          result += " • " + key + " is " + (status == 0 ? "off" : "on") + "\n";
      } else if(key.toLowerCase() == 'humidity'){
         result += " • " + key + " is " + status + "%\n";
      } else if(key.toLowerCase() == 'temperature'){
        result += " • " + key + " is " + status + "°C\n";
//          var celcius = (status - 32) * 5 / 9;
//          result += " • " + key + " is " + celcius + "°C ("+status+"°F)\n";
      } else {
         result += " • " + key + " is " + status + "\n";
      }
    }
  }
  res.send(result);
}

app.get('/getrequests', function (req, res) {
  //to check requests
  var keys = storage.keys();
  var result = "";
  for (var i = 0; i < keys.length; i++) {
    var key = keys[i];
    if(key.indexOf("-requested") > -1){
      var status = storage.getItemSync(keys[i]);
      result += key.replace('-requested','') + ":" + status + "\n";
    }
  }

//  res.send('status of "1234" is ' + status);
  res.send(result);
});

app.listen(4242, function () {
  console.log('Check The Status is running on port 4242');
  storage.initSync();
  console.log('Storage initialised');

});
