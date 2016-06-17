$(function() {
    var socket = io.connect();

    // ノブの更新
    $('#knob').knob({
        'change' : function(value) {}
    });
// unko
    $('#knob2').knob({
        'change' : function(value) {}
    });

    $('#knob3').knob({
        'change' : function(value) {}
    });

    $('#knob4').knob({
        change : function(value) {
            console.log("change :" + value);
        },
        release : function(value) {
            console.log("release :" + value);
            var msg = new Object();
            msg.servo = value;
            socket.emit('message', { value: msg });
        },
        cancel : function() {
            console.log("cancel :", this);
        }
    });


    // WebSocketでの接続
    socket.on('connect', function(msg) {
        console.log("connect");
    });

    // メッセージを受けたとき
    socket.on('message', function(msg) {
        if (typeof msg.value.temp != undefined) {
            // ノブの更新
            $('#knob').val(msg.value.temp).trigger('change');
        }
        if (typeof msg.value.humi != undefined) {
            // ノブの更新
            $('#knob2').val(msg.value.humi).trigger('change');
        }
        if (typeof msg.value.light != undefined) {
            // ノブの更新
            $('#knob3').val(msg.value.light).trigger('change');
        }
        if (typeof msg.value.servo != undefined) {
            // ノブの更新
           $('#knob4').val(msg.value.servo).trigger('change');
        }
    });

    // メッセージを送る
    function SendMsg() {
        var msg = $('#message').val();
        // メッセージを送信する
        socket.emit('message', { value: msg });
        $('#message').val('');
    }
});

