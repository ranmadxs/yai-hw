function tankLevelCallback(str){
  try {
    //console.log(str); 
    jsonObj = JSON.parse(str);
    //console.log(jsonObj);
    //console.log(jsonObj["type"]);
    if(String(jsonObj["type"]) == "YAI_TANK_HEIGHT"){
      //console.log("XD5555"+jsonObj["distance"]);
      setHight(jsonObj["height"]);
      updateLookupTableValue(jsonObj["volume"]);
      updateChangeRateValue(jsonObj["water_flow"]);
    }
  } catch (e) {
  } 
}


$( document ).ready(function() {
    console.log( "tank Level init!" );
    setText("H2O Level");
    removeArrow(); 
    addWSCallback(tankLevelCallback);


    /*
    var tankWS = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
    tankWS.onopen = function () {
        console.log('Connected: ');
        $("#counterDiv").prepend("Tank WS Connected [OK]</br>");
        // Ejemplo 1, peticion desde cliente
        //(function scheduleRequest() {
        //	connection.send("");
        //	setTimeout(scheduleRequest, 100);
        //})();
      };
      
      tankWS.onerror = function (error) {
        console.log('Tank WebSocket Error ', error);
      };
      
      tankWS.onmessage = function (e) {
        //updateCounterUI(e.data);
        str = e.data;
        try {
            jsonObj = JSON.parse(str);
            console.log("XD");
        } catch (e) {
        }    
        //$("#counterDiv").prepend("<< " + e.data + "</br>");
      };
      
      tankWS.onclose = function () {
        console.log('Tank WebSocket connection closed');
        //$("#counterDiv").prepend("WS connection closed</br>");
      };
      */    
});