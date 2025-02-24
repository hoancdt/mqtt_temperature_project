#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <string.h>

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "HOAN/TEMP"  // Topic mà publisher gửi nhiệt độ CPU đến

// Hàm xử lý khi nhận được tin nhắn từ MQTT
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    // Kiểm tra nếu topic nhận được là "HOAN/TEMP"
    if (strcmp(msg->topic, MQTT_TOPIC) == 0) {
        // In ra dữ liệu nhận được (nhiệt độ CPU)
        printf("Nhận nhiệt độ CPU: %s°C\n", (char *)msg->payload);
    }
}

int main() {
    struct mosquitto *mosq;
    
    // Khởi tạo MQTT
    mosquitto_lib_init();
    
    // Tạo client MQTT
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Lỗi tạo MQTT client\n");
        return 1;
    }

    // Đăng ký callback để xử lý khi nhận được tin nhắn
    mosquitto_message_callback_set(mosq, on_message);

    // Kết nối đến MQTT broker
    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Không thể kết nối MQTT Broker\n");
        return 1;
    }

    // Subscribe vào topic "HOAN/TEMP"
    mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);

    printf("Đang lắng nghe dữ liệu nhiệt độ CPU từ topic: %s\n", MQTT_TOPIC);

    // Chạy vòng lặp nhận tin nhắn MQTT
    mosquitto_loop_forever(mosq, -1, 1);

    // Dọn dẹp khi thoát
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    
    return 0;
}

