// 아래 코드는 ESP8266기반 Wemos계열의 아두이노 보드를 기준으로 쓰여졌습니다.
// 업로드를 위해서는 ESP8266 보드 라이브러리를 추가로 다운로드한 후 'Wemos D1 R1' 보드를 선택하고 업로드하시면 됩니다
// 자세한 방법은 아래 링크를 참고해 주세요.
// http://andy-power.blogspot.com/2016/12/arduino-ide-esp8266-board.html

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// 와이파이 정보 입력
#define SSID1 "wifi"  //와이파이 이름
#define PASS1 "password"  //와이파이 비밀번호

#define SSID2 "wifi"  //와이파이2 - 첫번째 와이파이에 연결할 수 없을 시 시도합니다 - 사용하지 않을 경우 비워주셔도 됩니다.
#define PASS2 "password"
// 와이파이 정보 입력

const char* link1 = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=1120059000"; //기상정보 - 성동구
const char* link2 = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4113164000"; //기상정보 - 성남시
const char* link3 = "http://openapi.airkorea.or.kr/openapi/services/rest/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?serviceKey=ncSK6OKRIMOj8HCAtGfTJP89UWLn9ZIYzCe%2Fo3uJ9SY8m%2BR%2FwE6lShNfQ5MYTAt0BZuQhHn4T0ZxS4I%2Fx%2B2H6g%3D%3D&numOfRows=1&pageSize=1&pageNo=1&startPage=1&stationName=%EC%A2%85%EB%A1%9C%EA%B5%AC&dataTerm=DAILY&ver=1.3"; //성동구 미세먼지
const char* link4 = "http://openapi.airkorea.or.kr/openapi/services/rest/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?serviceKey=ncSK6OKRIMOj8HCAtGfTJP89UWLn9ZIYzCe%2Fo3uJ9SY8m%2BR%2FwE6lShNfQ5MYTAt0BZuQhHn4T0ZxS4I%2Fx%2B2H6g%3D%3D&numOfRows=1&pageSize=1&pageNo=1&startPage=1&stationName=%EC%8B%A0%ED%9D%A5&dataTerm=DAILY&ver=1.3"; //수정구 미세먼지

String payload, data;
uint8_t temp1=0, hum1=0, temp2=0, hum2=0;  //temp1/hum1은 서울시 성동구 온도(°C)/습도(%), temp2/hum2는 성남시 수정구 온도(°C)/습도(%)
uint8_t pm10_1=0, pm25_1=0, pm10_2=0, pm25_2=0;  //pm10_1, pm25_1는 각각 성동구의 PM10, PM2.5 농도(㎍/㎥), pm10_2, pm25_2는 각각 수정구의 PM10, PM2.5 농도(㎍/㎥)
String weather1="", weather2="";  //각각 서울시 성동구, 성남시 수정구의 날씨 (맑음, 구름 조금, 구름 많음, 흐림, 비, 눈/비, 눈)

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

void setup(){
	Serial.begin(115200);
	Serial.println();
	Serial.println("Initializing");

	WiFi.mode(WIFI_STA);
	WiFiMulti.addAP(SSID1, PASS1);
	WiFiMulti.addAP(SSID2, PASS2);

	while(WiFiMulti.run() != WL_CONNECTED){
		Serial.print(".");
		delay(500);
	}
	Serial.println();

	Serial.print("Connected: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void loop() {
	Serial.println();
	Serial.println();
	Serial.println();

	http.begin(link1);
	if (http.GET() == HTTP_CODE_OK) {
		payload = http.getString();
		payload = payload.substring(payload.indexOf("<data seq=\"0\">"), payload.indexOf("</data>"));

		data = payload.substring(payload.indexOf("<temp>")+6, payload.indexOf("</temp>"));
		temp1 = data.toInt();
		data = payload.substring(payload.indexOf("<reh>")+5, payload.indexOf("</reh>"));
		hum1 = data.toInt();
		weather1 = payload.substring(payload.indexOf("<wfKor>")+7, payload.indexOf("</wfKor>"));

		Serial.print("서울시 성동구 온도: ");
		Serial.println(temp1);
		Serial.print("서울시 성동구 습도: ");
		Serial.println(hum1);
		Serial.print("서울시 성동구 날씨: ");
		Serial.println(weather1);
		Serial.println();
	}else{
		Serial.println("HTTP Connection Error");
	}
	http.end();

	http.begin(link2);
	if (http.GET() == HTTP_CODE_OK) {
		payload = http.getString();
		payload = payload.substring(payload.indexOf("<data seq=\"0\">"), payload.indexOf("</data>"));

		data = payload.substring(payload.indexOf("<temp>")+6, payload.indexOf("</temp>"));
		temp2 = data.toInt();
		data = payload.substring(payload.indexOf("<reh>")+5, payload.indexOf("</reh>"));
		hum2 = data.toInt();
		weather2 = payload.substring(payload.indexOf("<wfKor>")+7, payload.indexOf("</wfKor>"));

		Serial.print("성남시 수정구 온도: ");
		Serial.println(temp2);
		Serial.print("성남시 수정구 습도: ");
		Serial.println(hum2);
		Serial.print("성남시 수정구 날씨: ");
		Serial.println(weather2);
		Serial.println();
	}else{
		Serial.println("HTTP Connection Error");
	}
	http.end();

	http.begin(link3);
	if (http.GET() == HTTP_CODE_OK) {
		payload = http.getString();
		payload = payload.substring(payload.indexOf("<item>"), payload.indexOf("</item>"));

		data = payload.substring(payload.indexOf("<pm10Value>")+11, payload.indexOf("</pm10Value>"));
		pm10_1 = data.toInt();
		data = payload.substring(payload.indexOf("<pm25Value>")+11, payload.indexOf("</pm25Value>"));
		pm25_1 = data.toInt();

		Serial.print("서울시 성동구 PM10 농도: ");
		Serial.println(pm10_1);
		Serial.print("서울시 성동구 PM2.5 농도: ");
		Serial.println(pm25_1);
		Serial.println();
	}else{
		Serial.println("HTTP Connection Error");
	}
	http.end();

	http.begin(link4);
	if (http.GET() == HTTP_CODE_OK) {
		payload = http.getString();
		payload = payload.substring(payload.indexOf("<item>"), payload.indexOf("</item>"));

		data = payload.substring(payload.indexOf("<pm10Value>")+11, payload.indexOf("</pm10Value>"));
		pm10_2 = data.toInt();
		data = payload.substring(payload.indexOf("<pm25Value>")+11, payload.indexOf("</pm25Value>"));
		pm25_2 = data.toInt();

		Serial.print("성남시 수정구 PM10 농도: ");
		Serial.println(pm10_2);
		Serial.print("성남시 수정구 PM2.5 농도: ");
		Serial.println(pm25_2);
		Serial.println();
	}else{
		Serial.println("HTTP Connection Error");
	}
	http.end();

	delay(10000);
}
