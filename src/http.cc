#include "common.h"
#include "http.h"
#include <fmt/core.h>
#include <httplib.h>
#include <openssl/sha.h>

std::string getDateFmt() {
  std::time_t currentTime = std::time(nullptr);
  std::tm *gmtTime = std::gmtime(&currentTime);
  std::stringstream dss;
  dss << std::put_time(gmtTime, "%a, %d %b %Y %H:%M:%S GMT");
  return dss.str();
}

std::string base64Encode(const unsigned char *data, size_t length) {
  BIO *bio = BIO_new(BIO_s_mem());
  BIO *base64 = BIO_new(BIO_f_base64());
  BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_push(base64, bio);
  BIO_write(bio, data, length);
  BIO_flush(bio);

  char *encodedData;
  long encodedLength = BIO_get_mem_data(bio, &encodedData);
  std::string encodedStr(encodedData, encodedLength);

  BIO_free_all(bio);

  return encodedStr;
}

std::string signWithPrivateKey(const std::string &filePath,
                               const std::string &data) {
  // Load private key from the file
  FILE *keyFile = fopen(filePath.c_str(), "r");
  if (!keyFile) {
    throw std::runtime_error("Unable to open private key file");
  }

  EVP_PKEY *privateKey =
      PEM_read_PrivateKey(keyFile, nullptr, nullptr, nullptr);
  fclose(keyFile);

  if (!privateKey) {
    throw std::runtime_error("Unable to read private key");
  }

  std::vector<unsigned char> signature;
  // Initialize the digest sign context
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  if (!mdctx) {
    std::cerr << "Unable to create EVP_MD_CTX." << std::endl;
    EVP_PKEY_free(privateKey);

    throw std::runtime_error("failed");
  }

  // Initialize signing operation (e.g., using SHA-256)
  if (EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, privateKey) != 1) {
    std::cerr << "EVP_DigestSignInit failed." << std::endl;
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(privateKey);

    throw std::runtime_error("failed");
  }

  // Update the context with data to sign
  if (EVP_DigestSignUpdate(mdctx, data.c_str(), data.size()) != 1) {
    std::cerr << "EVP_DigestSignUpdate failed." << std::endl;
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(privateKey);

    throw std::runtime_error("failed");
  }

  // Determine the required size of the signature
  size_t sigLen = 0;
  if (EVP_DigestSignFinal(mdctx, NULL, &sigLen) != 1) {
    std::cerr << "EVP_DigestSignFinal (size) failed." << std::endl;
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(privateKey);

    throw std::runtime_error("failed");
  }

  signature.resize(sigLen);

  if (EVP_DigestSignFinal(mdctx, signature.data(), &sigLen) != 1) {
    std::cerr << "EVP_DigestSignFinal failed." << std::endl;
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(privateKey);
    throw std::runtime_error("failed");
  }

err:
cleanup:
  EVP_MD_CTX_free(mdctx);
  EVP_PKEY_free(privateKey);

  return base64Encode(signature.data(), sigLen);
}


std::string sha256(std::string data) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, data.c_str(), data.size());
  SHA256_Final(hash, &sha256);

  return base64Encode(hash, SHA256_DIGEST_LENGTH);
}

APClient::APClient(std::string host) : instance(host), cli("https://" + host) {}
httplib::Result APClient::Get(std::string pathname) {
  auto date = getDateFmt();

  std::string sigheader = "(request-target) host date";
  std::string sigbody = fmt::format("(request-target): get {}\nhost: {}\ndate: {}", pathname, instance, date);

  std::string signature = signWithPrivateKey("user.key", sigbody);

  std::string signatureHeader = fmt::format(
      R"(keyId="{}",algorithm="rsa-sha256",headers="{}",signature="{}")",
      USERPAGE(ct->userid), sigheader, signature);

  std::cout << pathname << "\n";
  return cli.Get(pathname, {
      {"Accept", "application/activity+json"},
      {"Algorithm", "rsa-sha256"},
      {"Signature", signatureHeader},
      {"Date", date},
      {"User-Agent", SOFTWARE "-" VERSION_LONG " " + FMT("({})", ct->baseurl)},
  });
}

httplib::Result APClient::Post(std::string pathname, json data) {
  // add json-ld context 
  data["@context"] = ct->context;
  std::string payload = data.dump();

  std::string digest = "SHA-256="+sha256(payload);

  auto date = getDateFmt();
  std::string sigheader = "(request-target) host date digest";
  std::string sigbody = fmt::format("(request-target): post {}\nhost: {}\ndate: {}\ndigest: {}", pathname,
                  instance, date, digest);

  std::string signedDigest = signWithPrivateKey("user.key", sigbody);

  std::string signatureHeader = fmt::format(
      R"(keyId="{}",algorithm="rsa-sha256",headers="{}",signature="{}")",
      USERPAGE(ct->userid), sigheader, signedDigest);

  return cli.Post(pathname, {
      {"Accept", "application/activity+json"},
      {"Algorithm", "rsa-sha256"},
      {"Signature", signatureHeader},
      {"Digest", digest},
      {"Date", date},
      {"User-Agent", SOFTWARE "-" VERSION_LONG " " + FMT("({})", ct->baseurl)},
  }, payload, "application/activity+json");
}
