#include <time.h>
#include "accelerator/apis.h"
#include "test_define.h"

iota_client_service_t service;
struct timespec start_time, end_time;

#if defined(ENABLE_STAT)
#define TEST_COUNT 100
#else
#define TEST_COUNT 1
#endif

double diff_time(struct timespec start, struct timespec end) {
  struct timespec diff;
  if (end.tv_nsec - start.tv_nsec < 0) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec + 1000000000;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

void test_generate_address(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(api_generate_address(&service, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of generate_address: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

void test_get_tips_pair(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(api_get_tips_pair(&service, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of get_tips_pair: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

void test_get_tips(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(api_get_tips(&service, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of get_tips: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

void test_send_transfer(void) {
  const char* json =
      "{\"value\":100,"
      "\"message\":\"" TAG_MSG "\",\"tag\":\"" TAG_MSG
      "\","
      "\"address\":\"" TRYTES_81_1 "\"}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(api_send_transfer(&service, json, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of send_transfer: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

void test_get_transaction_object(void) {
  char* json_result;
  double sum = 0;

  clock_gettime(CLOCK_REALTIME, &start_time);
  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(
        api_get_transaction_object(&service, TRYTES_81_1, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of get_transaction_object: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

void test_find_transactions_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(
        api_find_transactions_by_tag(&service, TAG_MSG, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of find_transactions_by_tag: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

void test_find_transactions_obj_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    clock_gettime(CLOCK_REALTIME, &start_time);
    TEST_ASSERT_FALSE(
        api_find_transactions_obj_by_tag(&service, TAG_MSG, &json_result));
    clock_gettime(CLOCK_REALTIME, &end_time);
#if defined(ENABLE_STAT)
    printf("%lf\n", diff_time(start_time, end_time));
#endif
    sum += diff_time(start_time, end_time);
  }
  printf("Average time of find_tx_obj_by_tag: %lf\n", sum / TEST_COUNT);
  free(json_result);
}

int main(void) {
  UNITY_BEGIN();
  service.http.path = "/";
  service.http.host = IRI_HOST;
  service.http.port = IRI_PORT;
  service.http.api_version = 1;
  service.serializer_type = SR_JSON;
  iota_client_core_init(&service);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  RUN_TEST(test_generate_address);
  RUN_TEST(test_get_tips_pair);
  RUN_TEST(test_get_tips);
  RUN_TEST(test_send_transfer);
  RUN_TEST(test_get_transaction_object);
  RUN_TEST(test_find_transactions_by_tag);
  RUN_TEST(test_find_transactions_obj_by_tag);
  iota_client_core_destroy(&service);
  return UNITY_END();
}
