#include <pgmspace.h>

#define THINGNAME "Esp_32_TrabalhoIOT"

const char Wifi_SSID[] = "Redmi 9T";
const char Wifi_Senha[] = "11111111";

const char AWS_IOT_ENDPOINT[] = "//Host";

const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
//Certificado CA
-----END CERTIFICATE-----
)EOF";

const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
//Cetificado do Cliente
-----END CERTIFICATE-----
)KEY";

const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
//Chave Privada
-----END RSA PRIVATE KEY-----
)KEY";



