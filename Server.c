#include <mosquitto.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg) {

  char *string = (char *)msg->payload;
  printf("In %s: %s\n", msg->topic, string);
  struct mosquitto *mosq_pub;
  mosq_pub = mosquitto_new("pub-test", true, NULL);
  mosquitto_username_pw_set(mosq_pub, "mqtt_user", "qwerty");
  int rc = mosquitto_connect(mosq_pub, "10.100.77.66", 1883, 60);
  if (rc != 0) {
    printf("Client could not connect to broker! Error code: %d\n", rc);
    return;
  }

  mosquitto_publish(mosq_pub, NULL, "server_to_car", strlen(string) + 1, string,
                    0, false);
  return;
}

int main() {

  int rc, id = 12;

  mosquitto_lib_init();

  struct mosquitto *mosq_sub;

  mosq_sub = mosquitto_new("sub-test", true, &id);

  mosquitto_username_pw_set(mosq_sub, "mqtt_user", "qwerty");

  mosquitto_message_callback_set(mosq_sub, on_message);

  rc = mosquitto_connect(mosq_sub, "10.100.77.66", 1883, 60);

  if (rc != 0) {
    printf("Client could not connect to broker! Error code: %d\n", rc);
    return -1;
  }
  mosquitto_subscribe(mosq_sub, NULL, "car_to_server", 0);

  mosquitto_loop_start(mosq_sub);
  printf("Press enter to quit\n");
  getchar();
  mosquitto_loop_stop(mosq_sub, true);

  mosquitto_disconnect(mosq_sub);

  mosquitto_destroy(mosq_sub);

  mosquitto_lib_cleanup();

  return 0;
}
