'use strict';

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);


//********************************************************
var xhrRequest = function(url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function() {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function locationSuccess(pos) {
    var myAPIKey = "ebe0a78125281118a038b2a62aab07c8";

    // Construct URL
    var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
        pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;
    console.log("Lat = " + pos.coords.latitude + " Long = " + pos.coords.longitude);
    
    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET',
        function(responseText) {
            // responseText contains a JSON object with weather info
            var json = JSON.parse(responseText);
            console.log("Response Text: " + responseText);

            // Temperature in Kelvin requires adjustment
            var tempC = Math.round(json.main.temp - 273.15);
           

            // City
            var city = json.name;

            // Assemble dictionary using our keys
            var dictionary = {
                "WEATHER_TEMPERATURE_KEY": tempC, // + "\u00B0C",
                "WEATHER_CITY_KEY": city
            };

            // Send to Pebble
            console.log("Temperature C is " + tempC);
            console.log("City is " + city);
          
            Pebble.sendAppMessage(dictionary,
                function(e) {
                    console.log("Weather info sent to Pebble successfully!");
                },
                function(e) {
                    console.log("Error sending weather info to Pebble!");
                }
            );
        }
    );
}

function locationError(err) {
    console.warn('location error (' + err.code + '): ' + err.message);
    Pebble.sendAppMessage({
        "WEATHER_CITY_KEY": "Loc Unavailable",
        "WEATHER_TEMPERATURE_KEY": "N/A"
    });
}

var locationOptions = {
    "timeout": 15000,
    "maximumAge": 60000
};

Pebble.addEventListener("ready", function(e) {
    console.log("Ready msg rcvd: Phone-Watch Communication Established, GPS find initiated");
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);

});

Pebble.addEventListener("appmessage", function(e) {
    console.log("Appmessage msg rcvd: Phone-Watch Communication Passsed, GPS find initiated");
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
});
