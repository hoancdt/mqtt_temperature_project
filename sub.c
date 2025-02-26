#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <mysql/mysql.h>
#include <string.h>

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "HOAN/TEMP"

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "Admin@1234"  // Cập nhật mật khẩu của bạn
#define DB_NAME "sensor_data"

MYSQL *conn;

// Hàm kết nối MySQL
void connect_db() {
    conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "Lỗi khởi tạo MySQL!\n");
        exit(1);
    }
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Lỗi kết nối MySQL: %s\n", mysql_error(conn));
        exit(1);
    }
    printf("Kết nối MySQL thành công!\n");
}

// Hàm lưu nhiệt độ vào MySQL
void save_to_db(float temp) {
    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO cpu_temperature (temperature) VALUES (%.1f)", temp);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Lỗi lưu dữ liệu vào MySQL: %s\n", mysql_error(conn));
    } else {
        printf("Đã lưu nhiệt độ CPU vào MySQL: %.1f°C\n", temp);
    }
}

// Hàm xử lý khi nhận dữ liệu MQTT
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    if (strcmp(msg->topic, MQTT_TOPIC) == 0) {
        float temp = atof(msg->payload);
        printf("Nhận nhiệt độ CPU: %.1f°C\n", temp);
        save_to_db(temp);
    }
}

int main() {
    struct mosquitto *mosq;
    
    // Kết nối MySQL
    connect_db();
    
    // Khởi tạo MQTT
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Lỗi tạo MQTT client\n");
        return 1;
    }

    // Đăng ký callback nhận tin nhắn
    mosquitto_message_callback_set(mosq, on_message);

    // Kết nối MQTT Broker
    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Không thể kết nối MQTT Broker\n");
        return 1;
    }

    // Đăng ký lắng nghe topic
    mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);
    printf("Đang lắng nghe dữ liệu nhiệt độ CPU từ topic: %s\n", MQTT_TOPIC);

    // Chạy vòng lặp nhận dữ liệu
    mosquitto_loop_forever(mosq, -1, 1);

    // Dọn dẹp
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mysql_close(conn);
    return 0;
}
