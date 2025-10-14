#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <string>
#include <mosquitto.h>

class MqttClient {
public:
    MqttClient(const std::string& client_id, const std::string& broker, int port);
    ~MqttClient();

    bool connect();
    bool publish(const std::string& topic, const std::string& message, int qos = 1);
    void disconnect();

private:
    struct mosquitto* mosq_;
    std::string broker_;
    int port_;
    bool connected_;

    static void on_connect_callback(struct mosquitto* mosq, void* obj, int result);
    static void on_publish_callback(struct mosquitto* mosq, void* obj, int mid);
    static void on_disconnect_callback(struct mosquitto* mosq, void* obj, int rc);
};

#endif // MQTT_CLIENT_H
