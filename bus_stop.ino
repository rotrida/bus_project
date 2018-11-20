#include <ArduinoJson.h>
#include <https_client.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <lcd_helper.h>
#include <bus_stop.h>
#include <memory>

lcd_helper lcd(0x27, 16, 2);
ESP8266WiFiMulti WiFiMulti;
int last_wifi_status;

void setup() 
{
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("VM4039731", "Qn7tfmpg4Ljj");

    //Use predefined PINS consts
    Wire.begin(D2, D1);

    lcd.set_full_text("Initializing now");
    lcd.display();

    last_wifi_status = WL_DISCONNECTED;
}

void loop() 
{
    const auto wifi_status = WiFiMulti.run();
    
    if(wifi_status != WL_CONNECTED)
    {
        if(wifi_status != last_wifi_status)
        {
            last_wifi_status = wifi_status;
            lcd.set_full_text("Not connected yet mama");
            lcd.display();
        }
        
        delay(500);
        return;
    }

    if(wifi_status != last_wifi_status)
    {
        last_wifi_status = wifi_status;
        lcd.set_full_text("Connected to Wifi");
        lcd.display();
    }

    https_client bus_client("api.tfl.gov.uk/StopPoint/490005314S/Arrivals", 443);

    if(bus_client.execute())
    {
        lcd.clear();

        Serial.println(bus_client.get_response());
        bus_stop buses(bus_client.get_response());

        if(!buses.error())
        {
            const auto & buses_map = buses.next_buses();
            int counter = 0;
            for(auto itr : buses_map)
            {
                const auto & bus_ptr = itr.second;
                String display = bus_ptr->name() + " " + String(bus_ptr->seconds_to_arrive() / 60);
                
                lcd.set_text(counter++, display);

                if(counter==2)
                    break;
            }
        }
        else
        {
            Serial.println(buses.last_error_message());
            lcd.set_full_text(buses.last_error_message());
        }
    }
    else
    {
        lcd.set_full_text(client.get_last_error());
        Serial.println(client.get_last_error());
    }

    //https_client bus_client("api.tfl.gov.uk/StopPoint/490005314S/Arrivals", 443);

    //http://api.openweathermap.org/data/2.5/weather?q=Brentford,uk&appid=e81dfff0c815e561a9681428db1c3cd3&units=metric

    lcd.display();
    delay(30000);
}
