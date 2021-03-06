/* CHART */
var config = {
    type: 'line',
    data: {
        labels: [],
        datasets: [
            { label: 'Temperatura', borderColor: 'red', data: [] },
            { label: 'Umidade', borderColor: 'blue', data: [] },
            { label: 'Pressao', borderColor: 'green', data: [] },
        ]
    }
};
window.onload = function() {
    var ctx = document.getElementById('myChart').getContext('2d');
    window.myChart = new Chart(ctx, config);
};
/* CHART */

/* MQTT */
let mqtt;
let reconnectTimeout = 2000;
let host = "<YOUR BROKER'S IP HERE>";
let port = 9001;

function onFailure(message){
    console.log("Conexão falhou com o host!");
    setTimeout(MQTTconnect, reconnectTimeout);
}

var contador = 0;
var status;

function onMessageArrived(msg){
    let obj = JSON.parse(msg.payloadString);

    var now = new Date();
    config.data.labels.push(now.getHours().toString().padStart(2, '0') + ':' + now.getMinutes().toString().padStart(2, '0') + ':' + now.getSeconds().toString().padStart(2, '0'));

    config.data.datasets[0].data.push(parseFloat(obj.temperatura));
    config.data.datasets[1].data.push(parseFloat(obj.umidade));
    config.data.datasets[2].data.push(parseFloat(obj.pressao));

    window.myChart.update();
}

 function onConnect(){
    console.log("Conectado ao host!");
    mqtt.subscribe("bme280");
}

function MQTTconnect(){
    console.log("Conectando ao host...");
    mqtt = new Paho.MQTT.Client(host, port, "clientjs");
    var options = {
        timeout: 3,
        onSuccess: onConnect,
        onFailure: onFailure,
    };
    mqtt.onMessageArrived = onMessageArrived
    mqtt.connect(options);
}
MQTTconnect();
/* MQTT */