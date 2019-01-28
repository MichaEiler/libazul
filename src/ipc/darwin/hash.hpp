#pragma once

#include <CommonCrypto/CommonDigest.h>
#include <cstdint>
#include <errno.h>
#include <sasl/saslutil.h>
#include <stdexcept>
#include <string>

namespace impulso
{
    namespace ipc
    {
        namespace detail
        {
            std::string sha1_hash(const std::string& data)
            {
                std::string result;
                result.resize(CC_SHA1_DIGEST_LENGTH);

                CC_SHA1_CTX context;
                CC_SHA1_Init(&context);
                CC_SHA1_Update(&context, data.c_str(), data.size());
                CC_SHA1_Final(reinterpret_cast<unsigned char*>(&result[0]), &context);

                return result;
            }

            std::string base64_encode(const std::string& data)
            {
                std::string result;
                result.resize(data.size() * 8 / 6 + 10);
                unsigned result_length = 0;

                // TODO: replace this function in the future, sasl_encode64 is marked as deprecated
                const auto success = sasl_encode64(data.c_str(), data.size(), reinterpret_cast<char*>(&result[0]), result.size(), &result_length);
                if (success != 0) {
                    throw std::runtime_error("Error while calling sasl_encode64: " + std::to_string(errno));
                }

                result.resize(static_cast<std::size_t>(result_length));
                return result;
            }
        }
    }
}
