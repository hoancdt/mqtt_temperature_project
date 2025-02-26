#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <mysql/mysql.h>
#include <string.h>
#include <unistd.h>

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "HOAN/TEMP"

#define DB_HOST "localhost"
#define DB_USER "root"         // user MySQL 
#define DB_PASS "Admin@1234" // mật khẩu MySQL 
#define DB_NAME "sensor_data"  // Tên cơ sở dữ liệu MySQL

MYSQL *conn;

// Hàm đọc nhiệt độ CPU từ hệ thống
float get_cpu_temp() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) return -1;
    int temp;
    fscanf(fp, "%d", &temp);
    fclose(fp);
    return temp / 1000.0;  // Chuyển đổi từ m°C sang °C
}

// Hàm kết nối MySQL
void connect_db() {
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Lỗi kết nối MySQL: %s\n", mysql_error(conn));
        exit(1);
    }
}

// Hàm lưu nhiệt độ CPU vào MySQL
void save_to_db(float temp) {
    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO cpu_temperature (temperature) VALUES (%.1f)", temp);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Lỗi lưu dữ liệu vào MySQL: %s\n", mysql_error(conn));
    } else {
        printf("Đã lưu nhiệt độ CPU vào MySQL: %.1f°C\n", temp);
    }
}

int main() {
    struct mosquitto *mosq;
    char payload[128];

    // Kết nối MySQL
    connect_db();

    // Khởi tạo MQTT
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Lỗi tạo MQTT client\n");
        return 1;
    }

    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Không thể kết nối MQTT Broker\n");
        return 1;
    }

    while (1) {
        float temp = get_cpu_temp();

        if (temp == -1) {
            printf("Không thể lấy nhiệt độ CPU\n");
            break;
        }

        // Gửi nhiệt độ CPU lên MQTT
        snprintf(payload, sizeof(payload), "%.1f", temp);
        mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 0, false);
        printf("Gửi nhiệt độ CPU qua MQTT: %s°C\n", payload);

        // Lưu vào MySQL
        save_to_db(temp);

        sleep(5);  // Gửi dữ liệu mỗi 5 giây
    }

    // Dọn dẹp tài nguyên
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mysql_close(conn);

    return 0;
}
