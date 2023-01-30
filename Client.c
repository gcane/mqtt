#include <mosquitto.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg) {
  struct timeval now;
  gettimeofday(&now, NULL);
  char string[100] = "";
  sprintf(string, "%s - %lu", (char *)msg->payload, now.tv_usec);
  printf("In %s: %s\n", msg->topic, string);
}

int main() {

  int rc, id = 12;

  mosquitto_lib_init();

  struct mosquitto *mosq_sub;
  struct mosquitto *mosq_pub;

  mosq_sub = mosquitto_new("subscriber-test", true, &id);
  mosq_pub = mosquitto_new("pusblish_test", true, NULL);

  mosquitto_username_pw_set(mosq_sub, "mqtt_user", "qwerty");
  mosquitto_username_pw_set(mosq_pub, "mqtt_user", "qwerty");

  mosquitto_message_callback_set(mosq_sub, on_message);

  rc = mosquitto_connect(mosq_sub, "10.100.77.66", 1883, 60);
  if (rc != 0) {
    printf("Client could not connect to broker! Error code: %d\n", rc);
    return -1;
  }

  rc = mosquitto_connect(mosq_pub, "10.100.77.66", 1883, 60);
  if (rc != 0) {
    printf("Client could not connect to broker! Error code: %d\n", rc);
    return -1;
  }

  mosquitto_subscribe(mosq_sub, NULL, "server_to_car", 0);
  mosquitto_loop_start(mosq_sub);
  struct timeval now;
  while (1) {
    gettimeofday(&now, NULL);
    char string[100] = "";
    sprintf(string, "%lu", now.tv_usec);
    mosquitto_publish(mosq_pub, NULL, "car_to_server", strlen(string), string,
                      0, false);
    sleep(3);
  }
  printf("Press enter to quit\n");
  getchar();
  mosquitto_loop_stop(mosq_sub, true);

  mosquitto_disconnect(mosq_sub);
  mosquitto_disconnect(mosq_pub);

  mosquitto_destroy(mosq_sub);
  mosquitto_destroy(mosq_pub);

  mosquitto_lib_cleanup();

  return 0;
}
