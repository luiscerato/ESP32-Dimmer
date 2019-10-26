/*
This is a JavaScript (JS) file.
JavaScript is the programming language that powers the web.

To use this file, place the following <script> tag just before the closing </body> tag in your HTML file, making sure that the filename after "src" matches the name of your file...

    <script src="script.js"></script>

Learn more about JavaScript at https://developer.mozilla.org/en-US/Learn/JavaScript

When you're done, you can delete all of this grey text, it's just a comment.
*/

var Valor_Salidas = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]; //Valores de salidas
var TimeUpdate; //Timer de actualizacion
var nDec = 1;
var ServerIP;
var ws = null; //Websocket de comandos
var ws_open = false;
var last_timestamp = 0; //Ultima vez que se envió un dato

//Se ejecuta cada vez que se inicia
window.onload = function() {
  //Configura el timer
  TimeUpdate = setInterval(UpdTimer, 500);

  //Buscar IP del servidor
  ESPData("ip", SetIP);
}

function UpdTimer() {
  OpenWebsocket();
  UpdateVals();
}

function SetIP(xhttp) {
  var response = xhttp.responseText;
  ServerIP = response;
  document.getElementById("ip_address").innerHTML = "Web server IP Address: " + ServerIP;
}

function OpenWebsocket() {
  if (ws==null) {
		ws = new WebSocket("ws://" + ServerIP + "/cmds");
	}
	else {
	  if (ws.readyState == 3) { //closed
		ws = new WebSocket("ws://" + ServerIP + "/cmds");
	  }
  }
}

function CloseWebsocket(){
  ws.close();
}

function SendDataToESP(data){
  var timestamp = new Date().getTime();
  if (ws) {
    ws.send(data);
  }
  last_timestamp = timestamp;
}

//Actualiza los valores
function UpdateVals() {
  ESPData("outs", ESPParseOutData);
}


function ESPData(url, cFunction) {
  var xhttp;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      cFunction(this);
    }
  };
  xhttp.open("GET", url, true);
  xhttp.send();
}

function ESPParseOutData(xhttp) {
  var response = xhttp.responseText;
  var vals = JSON.parse(response);

  Valor_Salidas[0] = vals.out1;
  Valor_Salidas[1] = vals.out2;
  Valor_Salidas[2] = vals.out3;
  Valor_Salidas[3] = vals.out4;
  Valor_Salidas[4] = vals.out5;
  Valor_Salidas[5] = vals.out6;
  Valor_Salidas[6] = vals.out7;
  Valor_Salidas[7] = vals.out8;
  Valor_Salidas[8] = vals.out9;
  Valor_Salidas[9] = vals.out10;

  //Darle tiempo a que se generen los cambios desde el último comando enviado
  var timestamp = new Date().getTime();
  if ((timestamp - last_timestamp > 1000)) {
    Update_Data();
  }
}

function OnBtnOnOffClick(sender) {
  var btn=sender.id;
  var index = parseInt(btn.slice(-1)) - 1;
  var cmd = "out%index%=%value%";

  if (sender.value == "OFF" ) {
    val = 100;}
  else if (sender.value == "ON" ) {
    val = 0;}
  cmd = cmd.replace("%index%", index.toString());
  cmd = cmd.replace("%value%", val.toString());
  Valor_Salidas[index] = val;
  Update_Data();

  //ESPData("outs?" + cmd, ESPParseOutData);
  SendDataToESP(cmd);
  console.log(cmd);
}

function OnSliderChange(sender, force = false) {
  var slider=sender.id;
  var index = parseInt(slider.slice(-1)) - 1;
  var cmd = "out%index%=%value%";
  cmd = cmd.replace("%index%", index.toString());
  cmd = cmd.replace("%value%", sender.value);
  //document.getElementById("out" + index).innerHTML = sender.value + " %";
  Valor_Salidas[index] = sender.value;
  Update_Data();

  var timestamp = new Date().getTime();
  //Solo actualiza cada 150 ms
  if ((timestamp - last_timestamp > 50) || force) {
    //ESPData("outs?" + cmd, ESPParseOutData);
    SendDataToESP(cmd);
    console.log(cmd);
  }
}

function Update_Data() {
  for (var i=0; i < 10; i++ ) {
    document.getElementById("out" + (i+1)).innerHTML = Valor_Salidas[i] + " %";
    document.getElementById("bar_out" + (i+1)).value = Valor_Salidas[i];
    if (Valor_Salidas[i]) {
      document.getElementById("btn_out"+ (i+1)).value = "ON";
    }
    else {
      document.getElementById("btn_out"+ (i+1)).value = "OFF";
    }
  }
}


//Procesar eventos de los botones de comandos
function OnBtnCmdClick(sender) {
  var btn=sender.id;
  var index = parseInt(btn.slice(-1)) - 1;
  var cmd = "preset=%index%";

  cmd = cmd.replace("%index%", index.toString());

  SendDataToESP(cmd);
  console.log(cmd);
}

