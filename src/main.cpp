#include <Arduino.h>
#include <ESP8266WiFi.h>

// tambahkan ini
#include <ArduinoJson.h>

#include "AES.h"
#include "base64.h"
#include "AES_config.h"

byte my_iv[N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte key[] = {0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48};

const char *ssid = "####";
const char *password = "####";
const uint16_t port = 2004;
const char *host = "192.168.0.100";

WiFiClient client;

void connect_wifi()
{
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  delay(250);
}

void connect_server()
{
  while (!client.connect(host, port))
  {
    Serial.printf("\n[Connecting to %s ... ", host);
    delay(1000);
    return;
  }
  Serial.println("connected]");
  delay(1000);
}

uint8_t getrnd()
{
  uint8_t really_random = *(volatile uint8_t *)0x3FF20E44;
  return really_random;
}

void gen_iv(byte *iv)
{
  for (int i = 0; i < N_BLOCK; i++)
  {
    iv[i] = (byte)getrnd();
  }
}

String do_encrypt(String msg, byte *key)
{
  size_t encrypt_size_len = 2000; //缓存长度
  DynamicJsonDocument root(1024); //

  char *b64data = new char[encrypt_size_len];
  byte *cipher = new byte[encrypt_size_len];

  AES aes;

  aes.set_key(key, sizeof(key));
  gen_iv(my_iv);

  memset(b64data, 0, encrypt_size_len);

  //IVbase64
  base64_encode(b64data, (char *)my_iv, N_BLOCK);
  root["iv"] = String(b64data);

  memset(b64data, 0, encrypt_size_len);
  memset(cipher, 0, encrypt_size_len);

  //msg base64
  int b64len = base64_encode(b64data, (char *)msg.c_str(), msg.length());

  //AES128，IV，CBCpkcs7
  aes.do_aes_encrypt((byte *)b64data, b64len, cipher, key, 128, my_iv);
  aes.clean();

  memset(b64data, 0, encrypt_size_len);

  base64_encode(b64data, (char *)cipher, aes.get_size());
  root["msg"] = String(b64data);

  String JsonBuff;
  serializeJson(root, JsonBuff);
  root.clear();

  delete[] b64data;
  delete[] cipher;

  return JsonBuff;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Contoh penggunaan socket client");
  connect_wifi();
  connect_server();
}

void loop()
{
  if (client.connected())
  {
    Serial.print("[Sending a request]: ");
    String data = do_encrypt("Polinema Yes", key);
    client.print(data);
    Serial.println(data);
  }
  else
  {
    connect_server();
  }
  delay(3000);
}