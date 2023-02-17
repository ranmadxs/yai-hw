var counterDiv = document.getElementById('counterDiv');

/*
function updateCounterUI(counter)
{
	counterDiv.innerHTML = counter; 
}
*/

$( document ).ready(function() {
  console.log( "jquery ready!" );
  counterDiv = document.getElementById('counterDiv');

  var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

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
    $("#counterDiv").prepend("<< " + e.data + "</br>");
  };
  
  connection.onclose = function () {
    console.log('WebSocket connection closed');
    $("#counterDiv").prepend("WS connection closed</br>");
  };
});

