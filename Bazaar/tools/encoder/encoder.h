void GenerateKeyPair(void);
void GenerateKeyPair(std::string& public_key, std::string & private_key);
std::string Encode(std::string _data);
std::string Decode(std::string _data);

extern const std::string _gPrivateKey;
extern const std::string _gPublicKey;