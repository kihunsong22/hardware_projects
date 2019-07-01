url_string = window.location.href
url = new URL(url_string)
dev_num = url.searchParams.get("dev_num")
// console.log("dev_num: "+dev_num)
var data
res_empty = 1
rep_empty = 1

var ios = /iPad|iPhone|iPod/.test(navigator.userAgent) && !window.MSStream;  // true for IOS, false for everything else
if(ios){
    function convertDateForIos(date) {
        var arr = date.split(/[- :]/);
        date = new Date(arr[0], arr[1]-1, arr[2], arr[3], arr[4], arr[5]);
        return date;
    }
}



function httpGet(idx) {
    url = "http://iotsv.cafe24.com/api.php" + "?idx=" + idx
    httpRequest = new XMLHttpRequest()
    httpRequest.onreadystatechange = function () {
        // console.log("ready state change")
    }
    httpRequest.open('GET', url, false)
    httpRequest.send()
    return httpRequest.responseText
}


function setData(dev_num) {
    data = httpGet(dev_num)
    data = JSON.parse(data)

    document.getElementById("devnum").innerHTML = data.devices.dev_num
    var cur_onoff = data.devices.cur_status == "1" ? "ON" : "OFF"
    document.getElementById("cur_status").innerHTML = cur_onoff

    document.getElementById("hid_devnum").value = data.devices.dev_num
    // var onoffSet = data.devices.set_status == "1" ? "0" : "1"

    // document.getElementById("hid_onoff").value = !data.devices.set_status;  // true/false 값이 바로 db에 들어가게되서 안됨
    // var timeAgo = moment(data.devices.update_time).fromNow(true)  // moment.js보다 PHP함수가 더 직관적
    document.getElementById("last_online").innerHTML = data.devices.timepassed
    if (Math.floor((new Date().getTime() - new Date(data.devices.update_time).getTime()) / 1000) > 60) {
        document.getElementById("status_img").src = "images/con0.png"
    } else {
        document.getElementById("status_img").src = "images/con1.png"
        msg = "new Date: " + new Date().getTime() + "     data.devices.update_time: " + new Date(data.devices.update_time).getTime()
        // alert(msg);
    }
    if(ios){
        asdf = convertDateForIos(data.devices.update_time)
        qwer = new Date(data.devices.update_time)
        msg = "Date: " + qwer + "     iosFunc: " + asdf
    //    alert(msg);
    }

    timestamp = moment().format('YYYY-MM-DD HH:mm:ss')
    time = moment().format('HH:mm:ss')
}

function setSubmitButton() {
    res_input = document.getElementById("res_input").value
    rep_input = document.getElementById("rep_input").value
    control = 2

    if (document.getElementById("radio_on").checked == 1) {
        control = 1
    } else if (document.getElementById("radio_off").checked == 1) {
        control = 0
    }

    if (rep_input == "" && res_input == "") {
        if (control == 1) {
            document.getElementById("onoff").value = "Turn On"
        } else if (control == 0) {
            document.getElementById("onoff").value = "Turn Off"
        }
    } else if (rep_input == "") {
        document.getElementById('onoff').value = "예약"
    } else {
        document.getElementById('onoff').value = "반복"
    }
}

function setRep(me) {
    me.value = (me.value == '' ? time : me.value)
    res_input = document.getElementById("res_input")
    rep_input = document.getElementById("rep_input")

    if (res_input.value != "" && rep_input.value != "") {
        res_input.value = ""
    }
}

function setRes(me) {
    me.value = (me.value == '' ? timestamp : me.value)
    res_input = document.getElementById("res_input")
    rep_input = document.getElementById("rep_input")

    if (res_input.value != "" && rep_input.value != "") {
        rep_input.value = ""
    }
}

function removeRes(idx) {
    if (!confirm("삭제하시겠습니까?")) {
        return
    }

    xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {};
    xhttp.open("POST", "/api.php", false);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    body = "delete=1&idx=" + idx
    xhttp.send(body);

    console.log("HTTP request done: ")
    response = xhttp.responseText
    console.log(response)
}

setData(dev_num)
updateShowRes()
setInterval(function () {
    setData(dev_num)
    updateShowRes()
}, 1000)

function updateShowRes() {
    showRes = document.getElementById("showReserve")
    showRes.innerHTML = ""
    for (i = 0; i < data.reserve["length"]; i++) {
        idx = data.reserve[i].idx
        message = '<a onclick="removeRes(' + idx + ')" id="remButton">'
        if (data.reserve[i].timestamp != null) { // 예약
            message += "예약: "
            message += data.reserve[i].timestamp + " 에 "
            message += data.reserve[i].control == 1 ? "켜기" : "끄기"
            message += "<br>"

        } else if (data.reserve[i].time != null) { // 반복
            dayText = data.reserve[i].day
            dayText = dayText.replace("1", "월, ")
            dayText = dayText.replace("2", "화, ")
            dayText = dayText.replace("3", "수, ")
            dayText = dayText.replace("4", "목, ")
            dayText = dayText.replace("5", "금, ")
            dayText = dayText.replace("6", "토, ")
            dayText = dayText.replace("7", "일, ")

            if (data.reserve[i].day.length > 0) {
                dayText = dayText.slice(0, -2)
            }
            message += dayText
            message += " "
            message += data.reserve[i].time + " 에 "
            message += data.reserve[i].control == 1 ? "켜기" : "끄기"
            message += '</a>'
            message += "<br>"

        }
        showRes.innerHTML += message
    }
}