
// Creare Nonce (doar pentru smkex):

#include <openssl/rand.h>
int RAND_bytes(unsigned char *buf, int num);

//RAND_bytes() puts num cryptographically strong pseudo-random bytes into buf.
//returns 1 on success, 0 otherwise



// To send encripted message: len length of message from buf I guess.
ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
// Encrypt
    uint32_t max_ciphertext_length = len + EVP_MAX_IV_LENGTH + EVP_MAX_BLOCK_LENGTH + SESSION_TAG_LENGTH;

    smkex_pkt* ppkt = smkex_pkt_allocate(max_ciphertext_length);

    ppkt->type = TLV_TYPE_DATA;  // ppkt mai contine value(presupun ce se trimite) length (cat de lung este) send (unde trebuie trimis) si acest type
 
    // now also need to compute before this session_key and iv for this session, de unde vine aici sockfd,trebuie sa fie canalul de transmisie pentru session
    mp_aesgcm_encrypt(buf, len, mp_sockets[sockfd].session_key, mp_sockets[sockfd].iv,  ppkt->value, &ppkt->length);

    smkex_pkt_send(ppkt, sockfd, flags);
