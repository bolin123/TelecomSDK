#ifndef BASE64_H
#define BASE64_H

unsigned char * Base64Encode(const unsigned char *src, int len,
    int *out_len);
unsigned char * Base64Decode(const unsigned char *src, int len,
    int *out_len);


#endif // !BASE64_H
