# LoRa_GPS_Tracker

## 1. H/W 구성
- WizArduino M0 ETH (Arduino Board)
http://wizwiki.net/wiki/doku.php?id=osh:wizarduino_m0_eth:start

- NEO-6M-0-001 (GPS Module)
- TLT01CS1 (LoRa Module)

## 2. H/W 연결
NEO-6M-0-001 의 Tx 핀을 D3에 연결
TLT01CS1  Tx 핀을 D0에 연결, Rx  D1에 연결

## 3. Driver 구성
기존의 variant.cpp와 variant.h를 Upload된 파일로 
