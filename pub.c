#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <string.h>
#include <unistd.h>

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "HOAN/TEMP"  // Chỉ gửi nhiệt độ CPU

// Hàm đọc nhiệt độ CPU từ hệ thống
float get_cpu_temp() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) return -1;
    int temp;
    fscanf(fp, "%d", &temp);
    fclose(fp);
    return temp / 1000.0;  // Chuyển từ milli-degree Celsius (m°C) sang Celsius (°C)
}

int main() {
    struct mosquitto *mosq;
    char payload[128];

    // Khởi tạo MQTT
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Lỗi tạo MQTT client\n");
        return 1;
    }

    // Kết nối tới MQTT broker
    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Không thể kết nối MQTT Broker\n");
        return 1;
    }

    while (1) {
        // Đọc nhiệt độ CPU
        float temp = get_cpu_temp();

        if (temp == -1) {
            printf("Không thể lấy nhiệt độ CPU\n");
            break;  // Dừng nếu không lấy được nhiệt độ
        }

        // Gửi nhiệt độ CPU lên MQTT
        snprintf(payload, sizeof(payload), "%.1f", temp);
        mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 0, false);
        printf("Gửi nhiệt độ CPU: %s°C\n", payload);

        sleep(5);  // Gửi dữ liệu mỗi 5 giây
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

