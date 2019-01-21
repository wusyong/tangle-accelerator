#include "common_core.h"

int cclient_get_txn_to_approve(const iota_client_service_t* const service,
                               ta_get_tips_res_t* res) {
  retcode_t ret = RC_OK;
  get_transactions_to_approve_req_t* get_txn_req =
      get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res =
      get_transactions_to_approve_res_new();
  // The depth at which Random Walk starts. Mininal is 3, and max is 15.
  get_transactions_to_approve_req_set_depth(get_txn_req, 3);

  ret = iota_client_get_transactions_to_approve(service, get_txn_req,
                                                get_txn_res);
  if (ret != RC_OK) {
    get_transactions_to_approve_req_free(&get_txn_req);
    get_transactions_to_approve_res_free(&get_txn_res);
    return -1;
  }

  hash243_stack_push(&res->tips, get_txn_res->branch);
  hash243_stack_push(&res->tips, get_txn_res->trunk);

  get_transactions_to_approve_res_free(&get_txn_res);
  get_transactions_to_approve_req_free(&get_txn_req);
  return 0;
}

int cclient_get_tips(const iota_client_service_t* const service,
                     ta_get_tips_res_t* res) {
  retcode_t ret = RC_OK;
  get_tips_res_t* get_tips_res = get_tips_res_new();
  ret = iota_client_get_tips(service, get_tips_res);
  if (ret != RC_OK) {
    get_tips_res_free(&get_tips_res);
    res->tips = NULL;
    return -1;
  }
  res->tips = get_tips_res->hashes;
  get_tips_res->hashes = NULL;
  get_tips_res_free(&get_tips_res);
  return 0;
}

int ta_generate_address(const iota_client_service_t* const service,
                        ta_generate_address_res_t* res) {
  retcode_t ret = RC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[243];
  flex_trits_from_trytes(seed_trits, 243, (const tryte_t*)SEED, 81, 81);
  address_opt_t opt = {.security = 3, .start = 0, .total = 0};

  ret = iota_client_get_new_address(service, seed_trits, opt, &out_address);

  if (ret == RC_OK) {
    res->addresses = out_address;
  }
  return ret;
}

int ta_send_transfer(const iota_client_service_t* const service,
                     const ta_send_transfer_req_t* const req,
                     ta_send_transfer_res_t* res) {
  /* make bundle
   * pow
   * broadcast
   */
  return 0;
}

int ta_find_transactions_by_tag(const iota_client_service_t* const service,
                                const char* const req,
                                ta_find_transactions_res_t* res) {
  retcode_t ret = RC_OK;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();

  flex_trit_t tag_trits[NUM_TRITS_TAG];
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)req,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  hash81_queue_push(&find_tx_req->tags, tag_trits);
  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);

  if (ret == RC_OK) {
    res->hashes = find_tx_res->hashes;
    find_tx_res->hashes = NULL;
  }

  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

int ta_get_transaction_object(const iota_client_service_t* const service,
                              const char* const req,
                              ta_get_transaction_object_res_t* res) {
  if (res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  flex_trit_t* tx_trits;
  flex_trit_t hash_trits[NUM_TRITS_HASH];

  // get raw transaction data of transaction hashes
  get_trytes_req_t* get_trytes_req = get_trytes_req_new();
  get_trytes_res_t* get_trytes_res = get_trytes_res_new();
  if (get_trytes_req == NULL || get_trytes_res == NULL) {
    goto done;
  }

  flex_trits_from_trytes(hash_trits, NUM_TRITS_HASH, (const tryte_t*)req,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(&get_trytes_req->hashes, hash_trits);
  ret = iota_client_get_trytes(service, get_trytes_req, get_trytes_res);
  if (ret) {
    goto done;
  }

  // deserialize raw data to transaction object
  tx_trits = hash8019_queue_peek(get_trytes_res->trytes);
  if (tx_trits == NULL) {
    goto done;
  }

  res->txn = transaction_deserialize(tx_trits, 0);
  transaction_set_hash(res->txn, hash_trits);

done:
  get_trytes_req_free(&get_trytes_req);
  get_trytes_res_free(&get_trytes_res);
  return ret;
}

int ta_find_transactions_obj_by_tag(const iota_client_service_t* const service,
                                    const char* const req,
                                    ta_find_transactions_obj_res_t* res) {
  if (res == NULL) {
    return -1;
  }
  int ret = 0;
  char hash_trytes[NUM_TRYTES_HASH + 1];
  flex_trit_t* hash_trits;

  ta_find_transactions_res_t* hash_res = ta_find_transactions_res_new();
  ta_get_transaction_object_res_t* obj_res =
      ta_get_transaction_object_res_new();
  if (hash_res == NULL || obj_res == NULL) {
    goto done;
  }

  // get transaction hash
  ret = ta_find_transactions_by_tag(service, req, hash_res);
  if (ret) {
    goto done;
  }

  // get transaction obj
  for (hash_trits = hash243_queue_peek(hash_res->hashes); hash_trits != NULL;
       hash_trits = hash243_queue_peek(hash_res->hashes)) {
    flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, hash_trits,
                         NUM_TRITS_HASH, NUM_TRITS_HASH);
    hash243_queue_pop(&hash_res->hashes);
    hash_trytes[NUM_TRYTES_HASH] = '\0';
    ret = ta_get_transaction_object(service, hash_trytes, obj_res);
    if (ret) {
      break;
    }
    utarray_push_back(res->txn_obj, obj_res->txn);
  }

done:
  ta_find_transactions_res_free(&hash_res);
  ta_get_transaction_object_res_free(&obj_res);
  return ret;
}