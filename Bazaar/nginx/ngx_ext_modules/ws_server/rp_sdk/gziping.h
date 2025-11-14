#pragma once
#include <cryptopp/filters.h>  // for ArraySource, VectorSink
#include <cryptopp/gzip.h>     // for Gzip
#include <memory>
#include <string>

using namespace CryptoPP;

void Gziping(const std::string& in, std::vector<unsigned char>& out) {
    ArraySource ss(in, true, new Gzip(new VectorSink(out), 1));
}

void GzipingBin(const byte* in_data, size_t in_size, std::vector<uint8_t>& out) {
    // Use the vector's data() and size() members
    CryptoPP::ArraySource ss(in_data, in_size, true, new CryptoPP::Gzip(new CryptoPP::VectorSink(out)));
}