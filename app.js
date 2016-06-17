var express = require('express')
    , routes = require('./routes')
    , path = require('path')
    , serialport = require('serialport');

var app = express()
    , http = require('http')
    , server = http.createServer(app)
    , io = require('socket.io').listen(server);


SerialPort = serialport.SerialPort,
    portName = process.argv[2];

// If the user didn't give a serial port name, exit the program:
if (typeof portName === "undefined") {
  console.log("You need to specify the serial port when you launch this script, like so:\n");
  console.log("    node wsServer.js <portname>");
  console.log("\n Fill in the name of your serial port in place of <portname> \n");
  process.exit(1);
}

var sp = new serialport.SerialPort(portName, {
    baudRate: 9600,
    dataBits: 8,
    parity: 'none',
    stopBits: 1,
    flowControl: false,
    parser: serialport.parsers.readline("\n")
});

app.configure(function(){
    app.set('port', process.env.PORT || 3000);
    app.set('views', __dirname + '/views');
    app.set('view engine', 'ejs');
    app.use(express.favicon());
    app.use(express.logger('dev'));
    app.use(express.bodyParser());
    app.use(express.methodOverride());
    app.use(express.cookieParser('your secret here'));
    app.use(express.session());
    app.use(app.router);
    app.use(express.static(path.join(__dirname, '/public')));
});

app.configure('development', function(){
    app.use(express.errorHandler());
});

app.get('/', routes.index);

// クライアントの接続を待つ(IPアドレスとポート番号を結びつけます)
server.listen(app.get('port'));

// クライアントが接続してきたときの処理
io.sockets.on('connection', function(socket) {
    console.log("connection");

    // メッセージを受けたときの処理
    socket.on('message', function(data) {
        console.log(data);
//        console.log(data.value.servo);
        // つながっているクライアント全員に送信
        socket.broadcast.json.emit('message', { value: data.value });

        console.log('Client sent us: ' + data.value.servo);
        sp.write('\{\"servo\": ' + data.value.servo + '\}\n', function(err, results) {
            console.log('JSON written: ', results);
        });
    });

    // クライアントが切断したときの処理
    socket.on('disconnect', function(){
        console.log("disconnect");
    });

    // data for Serial port
    socket.on('sendSerial', function(data) {
        var receive = JSON.stringify(data);
        console.log('Client sent us: ' + receive);
        sp.write(receive, function(err, bytesWritten) {
            console.log('bytes written: ', bytesWritten);
        });
    });
});

// data from Serial port
sp.on('data', function(input) {
    console.log('input ->' + input);
    var buffer = new Buffer(input, 'utf8');
    var jsonData;
    try {
        jsonData = JSON.parse(buffer);
        console.log('temp: ' + jsonData.temp);
        console.log('humi: ' + jsonData.humi);
        console.log('light: ' + jsonData.light);
        console.log('servo: ' + jsonData.servo);
    } catch(e) {
        // データ受信がおかしい場合無視する
        console.log("Wrong JSON format.-> " + input);
//        return;
    }
    // つながっているクライアント全員に送信
    io.sockets.json.emit('message', { value: jsonData });
});

sp.on('close', function(err) {
    console.log('port closed');
});

sp.on('open', function(error) {
    console.log('Seriapport opened. baudrate: ' + sp.options.baudRate);
});

sp.on('error', function(err) {
    console.log('Serial port error: ' + error);
});

