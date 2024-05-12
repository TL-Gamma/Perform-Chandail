#ifndef BP220_GPS_H
#define BP220_GPS_H

#include <Arduino.h>
#include <vector>

struct GGAPositionData {
  String altitude;
  String time;
  String latitude;
  String indicLat;
  String longitude;
  String indicLng;
};

struct RMCPositionData {
  String speed;
  String date;
  String time;
  String latitude;
  String indicLat;
  String longitude;
  String indicLng;
};

class BP220_GPS {
public:
  BP220_GPS(uint8_t rxPin, uint8_t txPin);
  bool begin();
  String readNMEA(String mode);
  std::vector<String> extractNMEA(String data);
  void printSeparatedValues(String data);

  // Fonctions pour obtenir les données GGA
  String GGAget_Altitude();
  String GGAget_Time();
  String GGAget_Latitude();
  String GGAget_indicLat();
  String GGAget_longitude();
  String GGAget_indicLng();

  // Fonctions pour obtenir les données RMC
  String RMCget_Speed();
  String RMCget_Date();
  String RMCget_Time();
  String RMCget_Latitude();
  String RMCget_indicLat();
  String RMCget_longitude();
  String RMCget_indicLng();

private:
  HardwareSerial _serial;
  GGAPositionData _ggaData;
  RMCPositionData _rmcData;

  void updateGGAPositionData(String data);
  void updateRMCPositionData(String data);
};

#endif
