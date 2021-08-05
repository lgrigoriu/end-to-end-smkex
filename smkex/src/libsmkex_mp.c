EC_KEY* __new_key_pair(void) {
    // Initialize new curve
    EC_KEY* ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (ec_key == NULL) {
        fprintf(stderr, "Error: Could not create EC key.\n");
        return NULL;
    }

    // Generate local key pair
    int rc = EC_KEY_generate_key(ec_key);
    if (rc == 0) {
        fprintf(stderr, "Error: Could not generate ECDH key.\n");
        EC_KEY_free(ec_key);
        return NULL;
    }

    return ec_key;
}


..........

    EC_KEY* ec_key = __new_key_pair();
    if (ec_key == NULL) {
        fprintf(stderr, "Error: Could not generate local key pair.\n");
        goto err_connect;
    }
.............
     const EC_POINT* ec_pub_key = EC_KEY_get0_public_key(ec_key);
............
     // make nonce: 
RAND_bytes(mp_sockets[sockfd].session.local_nonce, SESSION_NONCE_LENGTH)
  //returns 1 if OK, 0 if not OK
  
//..................

//calculate common key       
