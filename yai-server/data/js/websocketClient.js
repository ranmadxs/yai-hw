var counterDiv = document.getElementById('counterDiv');
var wsCallbacks = [];
var logCountTotal = 150;
var logCount = 0;
function addWSCallback(funcionCall){
  wsCallbacks.push(funcionCall);
}
/*
function updateCounterUI(counter)
{
	counterDiv.innerHTML = counter; 
}
*/

$( document ).ready(function() {
  console.log( "wsClient init!" );
  counterDiv = document.getElementById('counterDiv');

  var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
  $("#clearBtn").click(function(){
    $("#counterDiv").empty();
    console.log("clear logs");
  });
  connection.onopen = function () {
    console.log('Connected: ');
    $("#counterDiv").prepend("WS Connected [OK]</br>");
    // Ejemplo 1, peticion desde cliente
    //(function scheduleRequest() {
    //	connection.send("");
    //	setTimeout(scheduleRequest, 100);
    //})();
  };
  
  connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
  };
  
  connection.onmessage = function (e) {
    //updateCounterUI(e.data);
    console.log('Server: ', e.data);
    wsCallbacks.forEach(callback => {
      callback(e.data);
    });
    logCount++;
    if(logCount > logCountTotal){
      //$("#counterDiv").empty();
      $("#counterDiv p").slice(10).remove();
      logCount = 0;
    }
    var currentDate = new Date();
    var dateStr =
    //  ("00" + (currentDate.getMonth() + 1)).slice(-2) + "/" +
    //  ("00" + currentDate.getDate()).slice(-2) + "/" +
    //  currentDate.getFullYear() + " " +
      ("00" + currentDate.getHours()).slice(-2) + ":" +
      ("00" + currentDate.getMinutes()).slice(-2) + ":" +
      ("00" + currentDate.getSeconds()).slice(-2);    
    $("#counterDiv").prepend("<p><b>"+dateStr + "</b> | " + e.data + "</p>");
  };
  
  connection.onclose = function () {
    console.log('WebSocket connection closed');
    $("#counterDiv").prepend("WS connection closed</br>");
  };
});

