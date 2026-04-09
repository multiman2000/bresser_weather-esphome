#include "bresser_weather.h"
#include "esphome/core/log.h"
#include <SPI.h>

namespace esphome
{
    namespace bresser_weather
    {

        static const char *const TAG = "bresser_weather";

        void BresserWeatherComponent::setup()
        {
            ESP_LOGI(TAG, "Setting up Bresser Weather Sensor Receiver");

            SPI.begin(23, 22, 19, 33);
            SPI.setFrequency(1000000);

            this->ws_.begin();
            ESP_LOGI(TAG, "Receiver initialized successfully");
        }

        void BresserWeatherComponent::loop()
        {
            // Clear all sensor data
            this->ws_.clearSlots();

            // Try to receive radio message (non-blocking)
            int decode_status = this->ws_.getMessage();

            if (decode_status == DECODE_OK)
            {
                const int i = 0;

                if (this->filter_enabled_ && this->ws_.sensor[i].sensor_id != this->filter_sensor_id_)
                {
                    ESP_LOGD(TAG, "Ignoring sensor ID %08X (filter: %08X)",
                             (unsigned int)this->ws_.sensor[i].sensor_id,
                             (unsigned int)this->filter_sensor_id_);
                    return;
                }

                if ((this->ws_.sensor[i].s_type == SENSOR_TYPE_WEATHER0) ||
                    (this->ws_.sensor[i].s_type == SENSOR_TYPE_WEATHER1) ||
                    (this->ws_.sensor[i].s_type == SENSOR_TYPE_WEATHER3) ||
                    (this->ws_.sensor[i].s_type == SENSOR_TYPE_WEATHER8))
                {
                    if (this->sensor_id_sensor_ != nullptr)
                    {
                        char id_str[16];
                        snprintf(id_str, sizeof(id_str), "%08X", (unsigned int)this->ws_.sensor[i].sensor_id);
                        this->sensor_id_sensor_->publish_state(id_str);
                    }

                    if (this->rssi_sensor_ != nullptr)
                    {
                        this->rssi_sensor_->publish_state(this->ws_.sensor[i].rssi);
                    }

                    if (this->battery_sensor_ != nullptr)
                    {
                        this->battery_sensor_->publish_state(!this->ws_.sensor[i].battery_ok);
                    }

                    if (this->ws_.sensor[i].w.temp_ok && this->temperature_sensor_ != nullptr)
                    {
                        this->temperature_sensor_->publish_state(this->ws_.sensor[i].w.temp_c);
                    }

                    if (this->ws_.sensor[i].w.humidity_ok && this->humidity_sensor_ != nullptr)
                    {
                        this->humidity_sensor_->publish_state(this->ws_.sensor[i].w.humidity);
                    }

                    if (this->ws_.sensor[i].w.wind_ok)
                    {
                        if (this->wind_gust_sensor_ != nullptr)
                        {
                            this->wind_gust_sensor_->publish_state(this->ws_.sensor[i].w.wind_gust_meter_sec);
                        }
                        if (this->wind_speed_sensor_ != nullptr)
                        {
                            this->wind_speed_sensor_->publish_state(this->ws_.sensor[i].w.wind_avg_meter_sec);
                        }
                        if (this->wind_direction_sensor_ != nullptr)
                        {
                            this->wind_direction_sensor_->publish_state(this->ws_.sensor[i].w.wind_direction_deg);
                        }
                    }

                    if (this->ws_.sensor[i].w.rain_ok && this->rain_sensor_ != nullptr)
                    {
                        this->rain_sensor_->publish_state(this->ws_.sensor[i].w.rain_mm);
                    }

                    if (this->ws_.sensor[i].w.uv_ok && this->uv_sensor_ != nullptr)
                    {
                        this->uv_sensor_->publish_state(this->ws_.sensor[i].w.uv);
                    }

                    if (this->ws_.sensor[i].w.light_ok && this->light_sensor_ != nullptr)
                    {
                        this->light_sensor_->publish_state(this->ws_.sensor[i].w.light_klx);
                    }

                    ESP_LOGD(TAG, "Data published: Temp=%.1f°C, Hum=%d%%, Wind=%.1f/%.1f m/s @ %.0f°, Rain=%.1fmm, UV=%.1f, Light=%.1fklx, RSSI=%.1fdBm, Battery=%s",
                             this->ws_.sensor[i].w.temp_c,
                             this->ws_.sensor[i].w.humidity,
                             this->ws_.sensor[i].w.wind_avg_meter_sec,
                             this->ws_.sensor[i].w.wind_gust_meter_sec,
                             this->ws_.sensor[i].w.wind_direction_deg,
                             this->ws_.sensor[i].w.rain_mm,
                             this->ws_.sensor[i].w.uv,
                             this->ws_.sensor[i].w.light_klx,
                             this->ws_.sensor[i].rssi,
                             this->ws_.sensor[i].battery_ok ? "OK" : "Low");
                }
            }

            delay(100);
        }

    }
}
