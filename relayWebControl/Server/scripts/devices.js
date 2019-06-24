
url_string = window.location.href
url = new URL(url_string)
dev_num = url.searchParams.get("dev_num")
console.log("dev_num: "+dev_num)
var data
res_empty = 1
rep_empty = 1

function httpGet(idx){
    url = "http://iotsv.cafe24.com/api.php" + "?idx=" + idx
    httpRequest = new XMLHttpRequest()
    httpRequest.onreadystatechange = function(){
        // console.log("ready state change")
    }
    httpRequest.open('GET', url, false)
    httpRequest.send()
    return httpRequest.responseText
}


function setData(dev_num){
    data = httpGet(dev_num)
    data = JSON.parse(data)
    // console.log(data)

    document.getElementById("devnum").innerHTML = data.devices.dev_num
    var cur_onoff = data.devices.cur_status == "1" ? "ON" : "OFF"
    document.getElementById("cur_status").innerHTML = cur_onoff
    var onoffButton = data.devices.set_status == "1" ? "Turn Off" : "Turn On"
    document.getElementById("onoff").value = onoffButton
    document.getElementById("hid_devnum").value = data.devices.dev_num
    var onoffSet = data.devices.set_status == "1" ? "0" : "1"
    document.getElementById("hid_onoff").value = onoffSet
    // document.getElementById("hid_onoff").value = !data.devices.set_status;  // true/false 값이 바로 db에 들어가게되서 안됨
    // var timeAgo = moment(data.devices.update_time).fromNow(true)  // moment.js보다 PHP함수가 더 직관적
    document.getElementById("last_online").innerHTML = data.devices.timepassed
    if(Math.floor( (new Date().getTime() - new Date(data.devices.update_time).getTime()) / 1000 ) > 60){
        document.getElementById("status_img").src = "images/con0.png"
    }else{
        document.getElementById("status_img").src = "images/con1.png"
    }

    //반복/예약 상태 출력

    timestamp = moment().format('YYYY-MM-DD hh:mm:ss')
    time = moment().format('hh:mm:ss')
}

function setRep(me){
    me.value=(me.value=='' ? time : me.value)
    res_input = document.getElementById("res_input")
    rep_input = document.getElementById("rep_input")

    if(res_input.value!="" && rep_input.value!=""){
        res_input.value = ""
    }
}

function setRes(me){
    me.value=(me.value=='' ? timestamp : me.value)
    res_input = document.getElementById("res_input")
    rep_input = document.getElementById("rep_input")

    if(res_input.value!="" && rep_input.value!=""){
        rep_input.value = ""
    }
}

function removeRes(idx) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {};
    xhttp.open("POST", "/index2.php", true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    body = "delete=1&idx=" + idx
    xhttp.send(body);
}

setData(dev_num)
setInterval(function(){
    setData(dev_num)
    updateShowRes()
}, 1000)

function updateShowRes(){
    removeButton = '<input type="submit" value="삭제" onclick="removeRes(IDX)" style="width: 10px; height:10px;">'
    showRes = document.getElementById("showReserve")
    showRes.innerHTML = ""
    for(i=0; i<data.reserve["length"]; i++){
        idx = data.reserve[i].idx
        message = '<input type="submit" value="삭제" onclick="removeRes(idx)" style="width: 10px; height:10px;">'
        if(data.reserve[i].timestamp != null){  // 예약
            message += "예약: "
            message += data.reserve[i].timestamp + " 에 "
            message += data.reserve[i].control == 1 ? "켜기" : "끄기"
            message += "<br>"
            // console.log(message)

        }else{  // 반복
            message += "반복: "
            //day contains
            message += data.reserve[i].time + " 에 "
            message += data.reserve[i].control == 1 ? "켜기" : "끄기"
            message += "<br>"
            // console.log(message)
        }
        showRes.innerHTML += message
    }
}

// showRes.innerHTML += "<br>123";
